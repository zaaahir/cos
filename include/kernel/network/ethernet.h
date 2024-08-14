#include "drivers/network/networkdriver.h"
#include "common/hashmap.h"

#ifndef ETHERNET_H
#define ETHERNET_H

namespace Networking 
{
    namespace Ethernet
    {
        struct EthernetFrameHeader
        {
            uint64_t destinationMACBigEndian : 48;
            uint64_t sourceMACBigEndian : 48;
            uint16_t etherTypeBigEndian;
        } __attribute__ ((packed));

        class EthernetFrameHandler;

        class EthernetLayerManager : Drivers::EthernetNetworkDriverEventHandler
        {
        friend class EthernetFrameHandler;
        public:
            EthernetLayerManager(Drivers::EthernetNetworkDriver* driver);
            void on_receive_data(uint8_t* buf, uint64_t size);
            void send_data(uint64_t destinationMACBigEndian, uint16_t etherTypeBigEndian, uint8_t* buf, uint64_t size);
            uint64_t get_MAC_address();
            uint64_t get_MAC_address_Big_Endian();
            uint32_t get_IP_address();
            static EthernetLayerManager* instance();
        protected:
            //static EthernetLayerManager* m_self;
            Common::Hashmap<uint16_t, EthernetFrameHandler*> m_handlers;
        };

        // The EthernetFrameHandler class is abstract, and automatically registers itself with the EthernetLayerManager when constructed.
        class EthernetFrameHandler
        {
        friend class EthernetLayerManager;
        protected:
            EthernetFrameHandler(EthernetLayerManager* ethernetLayerManager, uint16_t etherType);
            ~EthernetFrameHandler();
            virtual void on_receive_frame(uint8_t* payload, uint32_t size) = 0;
            EthernetLayerManager* m_ethernetLayerManager;
            uint16_t m_etherTypeBigEndian;
        };
    }
}

#endif
