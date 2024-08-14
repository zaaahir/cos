#ifndef IPV4_H
#define IPV4_H

#include "network/ethernet.h"

namespace Networking
{
    namespace InternetProtocolV4
    {
        using InternetProtocolV4Address = uint32_t;

        class InternetProtocolHandler;

        class InternetProtocolManager : Ethernet::EthernetFrameHandler
        {
        friend class InternetProtocolHandler;
        public:
            InternetProtocolManager(Ethernet::EthernetLayerManager* ethernetLayerManager, InternetProtocolV4Address gatewayIP, uint32_t subnetMask);
            void on_receive_frame(uint8_t* payload, uint32_t size);
            void send_data(uint32_t destinationIPAddressBigEndian, uint8_t IPProtocol, uint8_t* payload, uint32_t size);
            uint16_t checksum(uint16_t* data, uint32_t length);
            static InternetProtocolManager* instance();
        private:
            Common::Hashmap<uint32_t, InternetProtocolHandler*> m_handlers;
            InternetProtocolV4Address m_gatewayIP;
            uint32_t m_subnetMask;
            static InternetProtocolManager* m_self;
        };

        class InternetProtocolHandler
        {
        friend class InternetProtocolManager;
        public:
            InternetProtocolHandler(InternetProtocolManager* manager, uint8_t IPProtocol);
            ~InternetProtocolHandler();
            virtual void on_receive_ip(InternetProtocolV4Address sourceIPAddressBigEndian, InternetProtocolV4Address destinationIPAddressBigEndian, uint8_t* payload, uint32_t size) = 0;
        private:
            InternetProtocolManager* m_internetProtocolManager;
            uint8_t m_IPProtocol;
        };

        class InternetProtocolV4Message
        {
        public:
            uint8_t get_version() { return (m_versionAndIHL >> 4) & 0xF; }
            void set_version(uint8_t version) { m_versionAndIHL = (m_versionAndIHL & 0x0F) | (version << 4); }
            uint8_t get_ihl() { return m_versionAndIHL & 0xF; }
            void set_ihl(uint8_t ihl) { m_versionAndIHL = (m_versionAndIHL & 0xF0) | ihl; }
            uint16_t get_length() { return m_length; };
            void set_length(uint16_t length) { m_length = length; }
            uint16_t get_ident() { return m_ident; }
            void set_ident(uint16_t ident) { m_ident = ident; }
            uint8_t get_ttl() { return m_ttl; }
            void set_ttl(uint8_t ttl) { m_ttl = ttl; }
            uint8_t get_protocol() { return m_protocol; }
            void set_protocol(uint8_t protocol) { m_protocol = protocol; }
            uint16_t get_header_checksum() { return m_headerChecksum; }
            void set_header_checksum(uint16_t headerChecksum) { m_headerChecksum = headerChecksum; }
            InternetProtocolV4Address get_source_ip() { return m_sourceAddress; }
            void set_source_ip(InternetProtocolV4Address address) { m_sourceAddress = address; }
            InternetProtocolV4Address get_dest_ip() { return m_destAddress; }
            void set_dest_ip(InternetProtocolV4Address address) { m_destAddress = address; }
            uint16_t get_flags() { return m_flagsAndFragment; }
            void set_flags(uint16_t flags) { m_flagsAndFragment = flags; }

        private:
            uint8_t m_versionAndIHL;
            uint8_t m_DSCPAndECN;
            uint16_t m_length;
            uint16_t m_ident;
            uint16_t m_flagsAndFragment;
            uint8_t m_ttl;
            uint8_t m_protocol;
            uint16_t m_headerChecksum;
            InternetProtocolV4Address m_sourceAddress;
            InternetProtocolV4Address m_destAddress;
        } __attribute__((packed));
    }
}

#endif
