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

        for (uint64_t i = 0; i < RECEIVE_BUFFER_COUNT; i++)
        {
            m_receiveBufferDescriptorRingBase[i].RMD0 = reinterpret_cast<uint64_t>(Memory::VirtualAddress(&m_receiveBuffersBase[i*BUFFER_SIZE]).get_low_physical().get());
            m_receiveBufferDescriptorRingBase[i].RMD1 = OWN_BUFFER | ONES | BUFFER_LEN;
            m_receiveBufferDescriptorRingBase[i].RMD2 = 0;
            m_receiveBufferDescriptorRingBase[i].RMD3 = 0;
        }

        auto initBlockPhysicalAddress = reinterpret_cast<uint64_t>(Memory::VirtualAddress(&m_initBlock).get_low_physical().get());

        // Initialise with initialisation block.
        IO::out_16(m_rapPort, CSR1);
        IO::out_16(m_rdpPort, (initBlockPhysicalAddress & 0xFFFF));
        IO::out_16(m_rapPort, CSR2);
        IO::out_16(m_rdpPort, ((initBlockPhysicalAddress >> 16) & 0xFFFF));
        IO::out_16(m_rapPort, CSR0);
        IO::out_16(m_rdpPort, CSR0_IENA | CSR0_INIT);

        IO::out_16(m_rapPort, CSR4);
        uint32_t csr4Contents = IO::in_16(m_rdpPort);
        IO::out_16(m_rapPort, CSR4);
        IO::out_16(m_rdpPort, csr4Contents | 0xC00);

        // Start receiving with interrupts enabled.
        IO::out_16(m_rapPort, CSR0);
        IO::out_16(m_rdpPort, CSR0_IENA | CSR0_STRT);
    }

    void AMDPCNetIIIDriver::irq_handler()
    {
        IO::out_16(m_rapPort, 0);
        uint32_t code = IO::in_16(m_rdpPort);

        if((code & 0x0400) == 0x0400) { receive_data(); }

        IO::out_16(m_rapPort, 0);
        IO::out_16(m_rdpPort, code);
    }

    void AMDPCNetIIIDriver::send_data(uint8_t* buf, uint64_t size)
    {
        IO::wait();
        uint8_t descriptor = m_currentTransmitBufferDescriptor;
        // Move to next transmit buffer.
        m_currentTransmitBufferDescriptor = (m_currentTransmitBufferDescriptor + 1) % TRANSMIT_BUFFER_COUNT;
        if (size > 1518) { printf("AMD PCNET ERROR: Trying to transmit frame of size greater than 1518"); }
        memcpy(Memory::VirtualAddress(Memory::PhysicalAddress(m_transmitBufferDescriptorRingBase[descriptor].TMD0)).get(), buf, size);
        m_transmitBufferDescriptorRingBase[descriptor].TMD1 = OWN_BUFFER | ONES | STP | ENP | ((uint16_t)((-size) & 0xFFF));
        m_transmitBufferDescriptorRingBase[descriptor].TMD2 = 0;
        m_transmitBufferDescriptorRingBase[descriptor].TMD3 = 0;
        IO::out_16(m_rapPort, CSR0);
        IO::out_16(m_rdpPort, CSR0_IENA | CSR0_TDMD);
    }

    void AMDPCNetIIIDriver::receive_data()
    {
        // Iterate through the circular buffer descriptors.
        for (; (m_receiveBufferDescriptorRingBase[m_currentReceiveBufferDescriptor].RMD1 & OWN_BUFFER) == 0; m_currentReceiveBufferDescriptor = (m_currentReceiveBufferDescriptor+1)%RECEIVE_BUFFER_COUNT)
        {
            // Find the buffer that has been written to by the device.
            if ((m_receiveBufferDescriptorRingBase[m_currentReceiveBufferDescriptor].RMD1 & (STP | ENP)) == (STP | ENP))
            {
                uint32_t size = m_receiveBufferDescriptorRingBase[m_currentReceiveBufferDescriptor].RMD1 & 0xFFF;
                uint8_t* buf = (uint8_t*)m_receiveBufferDescriptorRingBase[m_currentReceiveBufferDescriptor].RMD0;
                // The driver must remove the Ethernet frame check sequence, and discard any Ethernet frames which do not have the correct FCS.
                if (size > 64) { size -= 4; }

                auto eventMessage = Events::EventMessage();
                auto networkDataReceived = new NetworkDataReceived();
                networkDataReceived->buf = buf;
                networkDataReceived->size = size;
                eventMessage.message = (void*)networkDataReceived;
                Events::EventDispatcher::instance().dispatch_event(eventMessage, &m_self->m_listenqueue);
                Events::EventDispatcher::instance().wake_processes(&m_self->m_waitqueue);
            }
            m_receiveBufferDescriptorRingBase[m_currentReceiveBufferDescriptor].RMD1 = OWN_BUFFER | ONES | BUFFER_LEN;
        }
    }

    void AMDPCNetIIIDriver::block_event_listen(uint64_t ped, uint64_t tid)
    {
        Events::EventDispatcher::instance().add_process_to_wait_event_list(ped, tid, &m_waitqueue);
    }

    void AMDPCNetIIIDriver::register_event_listener(uint64_t ped, uint64_t tid)
    {
        Events::EventDispatcher::instance().add_process_to_listener_event_list(ped, tid, &m_listenqueue);
    }

    uint64_t AMDPCNetIIIDriver::get_MAC_address() { return m_MACAddress; }
    uint32_t AMDPCNetIIIDriver::get_IP_address() { return m_IPAddress; }
    void AMDPCNetIIIDriver::set_IP_address(uint32_t IPAddress) { m_IPAddress = IPAddress; }
}
