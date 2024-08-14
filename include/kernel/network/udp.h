#ifndef UDP_H
#define UDP_H

#include "network/ipv4.h"

namespace Networking
{
    namespace UserDatagramProtocol
    {
        using InternetProtocolV4Address = uint32_t; 

        class UserDatagramProtocolHeader
        {
        public:
            uint16_t get_source_port() { return m_sourcePort; }
            void set_source_port(uint16_t sourcePort) { m_sourcePort = sourcePort; }
            uint16_t get_dest_port() { return m_destPort; }
            void set_dest_port(uint16_t destPort) { m_destPort = destPort; }
            uint16_t get_length() { return m_length; }
            void set_length(uint16_t length) { m_length = length; }
            uint16_t get_checksum() { return m_checksum; }
            void set_checksum(uint16_t checksum) { m_checksum = checksum; }
        private:
            uint16_t m_sourcePort;
            uint16_t m_destPort;
            uint16_t m_length;
            uint16_t m_checksum;
        } __attribute__((packed));

        class UserDatagramProtocolSocket;

        class UserDatagramProtocolHandler
        {
        public:
            UserDatagramProtocolHandler() = default;
            virtual void handle_UDP_message(UserDatagramProtocolSocket* socket, uint8_t* data, uint16_t size) = 0;
        };

        class UserDatagramProtocolSocket
        {
        friend class UserDatagramProtocolManager;
        private:
            uint16_t m_remotePort = 0;
            InternetProtocolV4Address m_remoteIP = 0;
            uint16_t m_localPort = 0;
            InternetProtocolV4Address m_localIP = 0;
            bool m_listening = false;
            UserDatagramProtocolHandler* m_handler;
        public:
            UserDatagramProtocolSocket() = default;
            void receive_UDP_message(uint8_t* data, uint16_t size);
        };

        class UserDatagramProtocolManager : InternetProtocolV4::InternetProtocolHandler
        {
        public:
            UserDatagramProtocolManager();
            void on_receive_ip(InternetProtocolV4Address sourceIPAddressBigEndian, InternetProtocolV4Address destinationIPAddressBigEndian, uint8_t* payload, uint32_t size);
            // Connect to a remote endpoint.
            UserDatagramProtocolSocket* connect(InternetProtocolV4Address IPAddress, uint16_t port);
            // Listen for connections on a port.
            UserDatagramProtocolSocket* listen(uint16_t port);
            void disconnect(UserDatagramProtocolSocket* socket);
            void send(UserDatagramProtocolSocket* socket, uint8_t* data, uint16_t size);
            // Add handler to a socket.
            void bind(UserDatagramProtocolSocket* socket, UserDatagramProtocolHandler* handler);
        private:
            Common::DoublyLinkedList<UserDatagramProtocolSocket*> m_sockets;
            uint16_t m_freePort = 1024;
        };
    }
}

#endif
