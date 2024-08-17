#include "network/ethernet.h"

namespace Networking
{
    namespace Ethernet
    {
        static EthernetLayerManager* m_self;
        EthernetLayerManager* EthernetLayerManager::instance() { return m_self; }

        EthernetLayerManager::EthernetLayerManager(Drivers::EthernetNetworkDriver* driver) : Drivers::EthernetNetworkDriverEventHandler(driver) { m_self = this; }

        void EthernetLayerManager::on_receive_data(uint8_t* buf, uint64_t size)
        {
            auto frame = (EthernetFrameHeader*)(buf + KERNEL_V_BASE);
            // If the frame is intended for us, or is a broadcast frame, find the EtherType handler and pass the frame on.
            if (frame->destinationMACBigEndian == m_driver->get_MAC_address() || frame->destinationMACBigEndian == 0xFFFFFFFFFFFF)
            {
                auto it = m_handlers.find(frame->etherTypeBigEndian);
                if (!it.is_end())
                {
                    it->last->on_receive_frame(buf + KERNEL_V_BASE + sizeof(EthernetFrameHeader), size - sizeof(EthernetFrameHeader));
                }
            }
        }

        void EthernetLayerManager::send_data(uint64_t destinationMACBigEndian, uint16_t etherTypeBigEndian, uint8_t* buf, uint64_t size)
        {
            uint8_t* cpyBuf = (uint8_t*)kmalloc(sizeof(EthernetFrameHeader) + size, 0);
            EthernetFrameHeader* frame = (EthernetFrameHeader*)cpyBuf;

            frame->destinationMACBigEndian = destinationMACBigEndian;
            frame->sourceMACBigEndian = m_driver->get_MAC_address();
            frame->etherTypeBigEndian = etherTypeBigEndian;

            memcpy(cpyBuf+sizeof(EthernetFrameHeader), buf, size);
            m_driver->send_data(cpyBuf, sizeof(EthernetFrameHeader) + size);
            kfree(cpyBuf);
        }

        uint64_t EthernetLayerManager::get_MAC_address() { return m_driver->get_MAC_address(); }

        uint32_t EthernetLayerManager::get_IP_address() { return m_driver->get_IP_address(); }

        uint16_t switch_endian_16(uint16_t nb) {
            return (nb>>8) | (nb<<8);
        }

        uint32_t switch_endian_32(uint32_t nb) {
            return ((nb>>24)&0xff) | ((nb>>8)&0xff00) | ((nb<<8)&0xff0000) | ((nb<<24)&0xff000000);
        }

        uint64_t switch_endian_64(uint64_t nb) {
            return ((nb>>56)&0xff) | ((nb>>40)&0xff00) | ((nb>>24)&0xff0000) | ((nb>>8)&0xff000000)
            | ((nb<<8)&0xff00000000) | ((nb<<24)&0xff0000000000) | ((nb<<40)&0xff000000000000) | ((nb>>56)&0xff00000000000000);
        }

        uint64_t EthernetLayerManager::get_MAC_address_Big_Endian() { return switch_endian_64(m_driver->get_MAC_address()); }

        EthernetFrameHandler::EthernetFrameHandler(EthernetLayerManager* ethernetLayerManager, uint16_t etherType) : m_ethernetLayerManager(ethernetLayerManager),
                                                                                                                        m_etherTypeBigEndian(switch_endian_16(etherType))
        {
            m_ethernetLayerManager->m_handlers.insert(m_etherTypeBigEndian, this);
        }

        // Destructor                                                                                                                
        EthernetFrameHandler::~EthernetFrameHandler() { m_ethernetLayerManager->m_handlers.remove(m_etherTypeBigEndian); }
    }
}
