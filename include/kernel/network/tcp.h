#ifndef TCP_H
#define TCP_H

#include "network/ipv4.h"

namespace Networking
{
    namespace TransmissionControlProtocol
    {
        using InternetProtocolV4Address = uint32_t;

        class TransmissionControlProtocolPseudoHeader
        {
        public:
            uint32_t get_source_ip() { return m_sourceIP; }
            void set_source_ip(uint32_t ip) { m_sourceIP = ip; }
            uint32_t get_dest_ip() { return m_destIP; }
            void set_dest_ip(uint32_t ip) { m_destIP = ip; }
            uint32_t get_length() { return m_length; }
            void set_length(uint16_t length) { m_length = length; }
            uint16_t get_protocol() { return m_protocol; }
            void set_protocol(uint16_t protocol) { m_protocol = protocol; }
        private:
            uint32_t m_sourceIP;
            uint32_t m_destIP;
            uint16_t m_protocol;
            uint16_t m_length;
        } __attribute__((packed)) ;
       };
    }
}

#endif
