#include "drivers/network/amdpcnetiii.h"
#include "cpu/io/pci.h"

// This is a driver for the AMD Am79C973 - see (https://www.amd.com/content/dam/amd/en/documents/archived-tech-docs/datasheets/21510.pdf) for the technical specification

namespace Drivers
{
    AMDPCNetIIIDriver* AMDPCNetIIIDriver::m_self;
    AMDPCNetIIIDriver* AMDPCNetIIIDriver::instance() { return m_self; }

    AMDPCNetIIIDriver::AMDPCNetIIIDriver() : m_PCIDevice(PeripheralComponentInterconnect::PCIManager::instance()->get_device(AMDPCNETIII_VENDOR_ID, AMDPCNETIII_DEVICE_ID)),
                                            m_rdpPort(m_PCIDevice->ioPortBase + RDP), m_rapPort(m_PCIDevice->ioPortBase + RAP), m_resetPort(m_PCIDevice->ioPortBase + RESET), m_bdpPort(m_PCIDevice->ioPortBase + BDP)
    {
        m_self = this;
        Events::EventDispatcher::instance().register_event_sender("Network/Event", this);
        m_currentTransmitBufferDescriptor = 0;
        m_currentReceiveBufferDescriptor = 0;
        // Register interrupt handler.
        update_handler_in_interrupt_table(m_PCIDevice->interruptLine + 32);

        // Enable I/O and bus mastering on PCI.
        uint32_t conf = PeripheralComponentInterconnect::PCIManager::instance()->read_dword(m_PCIDevice->bus, m_PCIDevice->device, m_PCIDevice->function, 0x4);
        conf &= 0xFFFF0000;
        conf |= 0x5;
        PeripheralComponentInterconnect::PCIManager::instance()->write_dword(m_PCIDevice->bus, m_PCIDevice->device, m_PCIDevice->function, 0x4, conf);

        // Read MAC address from device.
        m_MACAddress = 0;
        m_MACAddress = IO::in_16(m_PCIDevice->ioPortBase + APROM4);
        m_MACAddress <<= 16;
        m_MACAddress += IO::in_16(m_PCIDevice->ioPortBase + APROM2);
        m_MACAddress <<= 16;
        m_MACAddress += IO::in_16(m_PCIDevice->ioPortBase + APROM0);

        // Reset the device by reading from the RESET register
        IO::in_16(m_resetPort);
        // Enter 32 bit mode
        IO::out_16(m_rapPort, BCR20);
        IO::out_16(m_bdpPort, BCR20_SSIZE32 | BCR20_PCNET_PCI_32);
        // STOP reset
        IO::out_16(m_rapPort, CSR0);
        IO::out_16(m_rdpPort, 0x04);

        // Create initialisation block
        m_initBlock.resv1 = 0;
        m_initBlock.resv2 = 0;
        m_initBlock.resv3 = 0;
        // We have 8 transmit and receive buffers.
        m_initBlock.rlen = 3;
        m_initBlock.tlen = 3;
        m_initBlock.mode = 0x0000;
        m_initBlock.padr = m_MACAddress;
        m_initBlock.ladrf = 0;

        m_transmitBufferDescriptorRingBase = (TransmitDescriptor32*)(BYTE_ALIGN_UP(reinterpret_cast<uint64_t>(m_transmitBufferDescriptors), 16));
        m_receiveBufferDescriptorRingBase = (ReceiveDescriptor32*)(BYTE_ALIGN_UP(reinterpret_cast<uint64_t>(m_receiveBufferDescriptors), 16));
        
        // Get physical addresses of transmit and receive buffer descriptor rings.
        m_initBlock.tdra = reinterpret_cast<uint64_t>(Memory::VirtualAddress(m_transmitBufferDescriptorRingBase).get_low_physical().get());
        m_initBlock.rdra = reinterpret_cast<uint64_t>(Memory::VirtualAddress(m_receiveBufferDescriptorRingBase).get_low_physical().get());

        m_transmitBuffersBase = (uint8_t*) BYTE_ALIGN_UP(reinterpret_cast<uint64_t>(m_transmitBuffers), 16);
        m_receiveBuffersBase = (uint8_t*) BYTE_ALIGN_UP(reinterpret_cast<uint64_t>(m_receiveBuffers), 16);

        // Set up descriptors for transmit and receive buffers.
        for (uint64_t i = 0; i < TRANSMIT_BUFFER_COUNT; i++)
        {
            m_transmitBufferDescriptorRingBase[i].TMD0 = reinterpret_cast<uint64_t>(Memory::VirtualAddress(&m_transmitBuffersBase[i*BUFFER_SIZE]).get_low_physical().get());
            m_transmitBufferDescriptorRingBase[i].TMD1 = ONES | BUFFER_LEN;
            m_transmitBufferDescriptorRingBase[i].TMD2 = 0;
            m_transmitBufferDescriptorRingBase[i].TMD3 = 0;
        }
