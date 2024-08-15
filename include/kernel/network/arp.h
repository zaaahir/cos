#ifndef ARP_H
#define ARP_H

#include "types.h"
#include "network/ethernet.h"
#include "common/hashmap.h"

#define ARP_HTYPE_ETHERNET 0x0100
#define ARP_PTYPE_IPv4 0x0008
#define ARP_OP_REQ 0x0100
#define ARP_OP_REPLY 0x0200

#define ARP_ETHERTYPE 0x0806
#define ARP_BROADCAST_ADDRESS 0xFFFFFFFFFFFF

namespace Networking
{
    namespace AddressResolutionProtocol
    {
        struct AddressResolutionProtocolMessage
        {
            uint16_t hardwareType;
            uint16_t protocolType;
            uint8_t hardwareAddressLen;
            uint8_t protocolAddressLen;
            uint16_t operation;
            uint64_t senderHardwareAddress : 48;
            uint32_t senderProtocolAddress;
            uint64_t targetHardwareAdddress : 48;
            uint32_t targetProtocolAddress;
        } __attribute__((packed));

        class AddressResolutionProtocolManager : Ethernet::EthernetFrameHandler
        {
        public:
            AddressResolutionProtocolManager(Ethernet::EthernetLayerManager* ethernetLayerManager);
            //~AddressResolutionProtocolManager() = default;
            Common::Hashmap<uint32_t, uint64_t> m_MACCache;
            uint64_t find_cached_IP_address(uint32_t IPAddressBigEndian);
            void send_ARP_request(uint32_t IPAddressBigEndian);
            void on_receive_frame(uint8_t* payload, uint32_t size);
            void broadcast_MAC_address(uint32_t IPAddressBigEndian);
            static AddressResolutionProtocolManager* instance();
        private:
            static AddressResolutionProtocolManager* m_self;
            void handle_ARP_request(AddressResolutionProtocolMessage* message);
            void handle_ARP_reply(AddressResolutionProtocolMessage* message);
        };
    }
}

#endif
