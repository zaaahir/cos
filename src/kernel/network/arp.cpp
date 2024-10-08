#include "network/arp.h"

namespace Networking
{
    namespace AddressResolutionProtocol
    {
        AddressResolutionProtocolManager* AddressResolutionProtocolManager::m_self;
        AddressResolutionProtocolManager* AddressResolutionProtocolManager::instance() { return m_self; }
        AddressResolutionProtocolManager::AddressResolutionProtocolManager(Ethernet::EthernetLayerManager* ethernetLayerManager) : Ethernet::EthernetFrameHandler(ethernetLayerManager, ARP_ETHERTYPE) { m_self = this; }

        void AddressResolutionProtocolManager::on_receive_frame(uint8_t* payload, uint32_t size)
        {
            // Verify that the frame is targetting us and asks for an IPv4 to MAC address translation,
            if (size < sizeof(AddressResolutionProtocolMessage)) { return; }
            auto arpmsg = (AddressResolutionProtocolMessage*)payload;
            if (arpmsg->hardwareType == ARP_HTYPE_ETHERNET
                && arpmsg->protocolType == ARP_PTYPE_IPv4
                && arpmsg->hardwareAddressLen == 6
                && arpmsg->protocolAddressLen == 4
                && arpmsg->targetProtocolAddress == m_ethernetLayerManager->get_IP_address())
            {
                switch (arpmsg->operation)
                {
                    case ARP_OP_REQ:
                        handle_ARP_request(arpmsg);
                        break;
                    case ARP_OP_REPLY:
                        handle_ARP_reply(arpmsg);
                        break;
                }
            }
        }

        void AddressResolutionProtocolManager::handle_ARP_request(AddressResolutionProtocolMessage* message)
        {
            // Send a response with our MAC address.
            AddressResolutionProtocolMessage arpResponse(*message);
            arpResponse.operation = ARP_OP_REPLY;
            arpResponse.targetHardwareAdddress = message->senderHardwareAddress;
            arpResponse.targetProtocolAddress = message->senderProtocolAddress;
            arpResponse.senderHardwareAddress = m_ethernetLayerManager->get_MAC_address();
            arpResponse.senderProtocolAddress = m_ethernetLayerManager->get_IP_address();
            m_ethernetLayerManager->send_data(message->senderHardwareAddress, m_etherTypeBigEndian, (uint8_t*)&arpResponse, sizeof(AddressResolutionProtocolMessage));
        }

        void AddressResolutionProtocolManager::handle_ARP_reply(AddressResolutionProtocolMessage* message)
        {
            m_MACCache.insert(message->senderProtocolAddress, message->senderHardwareAddress);
        }

        void AddressResolutionProtocolManager::send_ARP_request(uint32_t IPAddressBigEndian)
        {
            // An ARP request is sent to the broadcast MAC address to target all devices on the network.
            AddressResolutionProtocolMessage arpmsg;
            arpmsg.hardwareType = ARP_HTYPE_ETHERNET;
            arpmsg.protocolType = ARP_PTYPE_IPv4;
            arpmsg.hardwareAddressLen = 6;
            arpmsg.protocolAddressLen = 4;
            arpmsg.operation = ARP_OP_REQ;
            arpmsg.senderHardwareAddress = m_ethernetLayerManager->get_MAC_address();
            arpmsg.senderProtocolAddress = m_ethernetLayerManager->get_IP_address();
            arpmsg.targetHardwareAdddress = ARP_BROADCAST_ADDRESS;
            arpmsg.targetProtocolAddress = IPAddressBigEndian;
            m_ethernetLayerManager->send_data(ARP_BROADCAST_ADDRESS, m_etherTypeBigEndian, (uint8_t*)&arpmsg, sizeof(AddressResolutionProtocolMessage));
        }

        uint64_t AddressResolutionProtocolManager::find_cached_IP_address(uint32_t IPAddressBigEndian)
        {
            auto it = m_MACCache.find(IPAddressBigEndian);
            if (!it.is_end())
            {
                return it->last;
            }
            return 0;
        }

        void AddressResolutionProtocolManager::broadcast_MAC_address(uint32_t IPAddressBigEndian)
        {
            AddressResolutionProtocolMessage message;
            message.hardwareType = 0x0100;
            message.protocolType = 0x0008;
            message.hardwareAddressLen = 6;
            message.protocolAddressLen = 4;
            message.operation = 0x0200;

            message.senderHardwareAddress = m_ethernetLayerManager->get_MAC_address();
            message.senderProtocolAddress = m_ethernetLayerManager->get_IP_address();
            message.targetHardwareAdddress = find_cached_IP_address(IPAddressBigEndian);
            if (!message.targetHardwareAdddress)
            {
                send_ARP_request(IPAddressBigEndian);
                while (!message.targetHardwareAdddress)
                {
                    message.targetHardwareAdddress = find_cached_IP_address(IPAddressBigEndian);
                }
            }
            message.targetProtocolAddress = IPAddressBigEndian;

            m_ethernetLayerManager->send_data(message.targetHardwareAdddress, 0x0608, (uint8_t*)&message, sizeof(AddressResolutionProtocolMessage));
        }
    }
}
