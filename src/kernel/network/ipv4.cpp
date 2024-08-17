#include "network/ipv4.h"
#include "network/arp.h"

namespace Networking
{
    namespace InternetProtocolV4
    {
        InternetProtocolManager* InternetProtocolManager::m_self;

        InternetProtocolManager* InternetProtocolManager::instance() { return m_self; }

        InternetProtocolHandler::InternetProtocolHandler(InternetProtocolManager* internetProtocolManager, uint8_t IPProtocol) : m_internetProtocolManager(internetProtocolManager), m_IPProtocol(IPProtocol)
        {
            m_internetProtocolManager->m_handlers.insert(m_IPProtocol, this);
        }

        InternetProtocolManager::InternetProtocolManager(Ethernet::EthernetLayerManager* ethernetLayerManager, InternetProtocolV4Address gatewayIP, uint32_t subnetMask) : EthernetFrameHandler(ethernetLayerManager, 0x800), m_gatewayIP(gatewayIP), m_subnetMask(subnetMask) { m_self = this; }

        InternetProtocolHandler::~InternetProtocolHandler() { m_internetProtocolManager->m_handlers.remove(m_IPProtocol); }

        void InternetProtocolManager::on_receive_frame(uint8_t* payload, uint32_t size)
        {
            if (size < sizeof(InternetProtocolV4Message)) { return; }

            auto IPMessage = (InternetProtocolV4Message*)payload;

            if (IPMessage->get_dest_ip() != m_ethernetLayerManager->get_IP_address()) { return; }

            auto length = (IPMessage->get_length()& 0xFF00) >> 8 | (IPMessage->get_length()& 0x00FF) << 8;
            if (length > size) { length = size; }

            // Find the handler for the IP protocol and pass the IP packet to it. This is the same data model used for EthernetFrameHandlers.
            auto handler = m_handlers.find(IPMessage->get_protocol());
            if (!handler.is_end())
            {
                handler->last->on_receive_ip(IPMessage->get_source_ip(), IPMessage->get_dest_ip(), payload + 4*IPMessage->get_ihl(), length - 4*IPMessage->get_ihl());
            }
        }

        void InternetProtocolManager::send_data(uint32_t destinationIPAddressBigEndian, uint8_t IPProtocol, uint8_t* payload, uint32_t size)
        {
            auto buffer = (uint8_t*)kmalloc(sizeof(InternetProtocolV4Message) + size, 0);
            auto message = (InternetProtocolV4Message*) buffer;
            message->set_version(4);
            message->set_ihl(sizeof(InternetProtocolV4Message)/4);
            message->set_length(size+sizeof(InternetProtocolV4Message));
            message->set_length(((message->get_length() & 0xFF00) >> 8)  | ((message->get_length() & 0x00FF) << 8));
            message->set_ident(0x0100);
            message->set_flags(0x40);
            message->set_ttl(0x40);
            message->set_protocol(IPProtocol);
            message->set_dest_ip(destinationIPAddressBigEndian);
            message->set_source_ip(m_ethernetLayerManager->get_IP_address());
            message->set_header_checksum(0);
            message->set_header_checksum(checksum((uint16_t*)message, sizeof(InternetProtocolV4Message)));
            memcpy(buffer+sizeof(InternetProtocolV4Message), payload, size);
            uint32_t route = destinationIPAddressBigEndian;

            if((destinationIPAddressBigEndian & m_subnetMask) != (message->get_source_ip() & m_subnetMask))
            {
                // Devices are not on the same subnet, send to gateway instead.
                route = m_gatewayIP;
            }
            uint64_t routeAddress = AddressResolutionProtocol::AddressResolutionProtocolManager::instance()->find_cached_IP_address(route);
            // Loop until the cache is updated with the ARP_request.
            if (!routeAddress)
            {
                AddressResolutionProtocol::AddressResolutionProtocolManager::instance()->send_ARP_request(route);
                while (!routeAddress)
                {
                    routeAddress = AddressResolutionProtocol::AddressResolutionProtocolManager::instance()->find_cached_IP_address(route);
                }
            }
            m_ethernetLayerManager->send_data(routeAddress, this->m_etherTypeBigEndian, buffer, sizeof(InternetProtocolV4Message) + size);
            kfree(buffer);
        }

        uint16_t InternetProtocolManager::checksum(uint16_t* data, uint32_t length)
        {
            // See InternetProtocolv4 Spec. for more details.
            uint32_t temp = 0;

            for(int i = 0; i < length/2; i++)
                temp += ((data[i] & 0xFF00) >> 8) | ((data[i] & 0x00FF) << 8);

            if(length % 2)
                temp += ((uint16_t)((char*)data)[length-1]) << 8;
            
            while(temp & 0xFFFF0000)
                temp = (temp & 0xFFFF) + (temp >> 16);
            
            return ((~temp & 0xFF00) >> 8) | ((~temp & 0x00FF) << 8);
        }
    }
}
