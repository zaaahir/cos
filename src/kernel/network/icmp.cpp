#include "network/icmp.h"

namespace Networking
{
    namespace InternetProtocolV4
    {
        InternetControlMessageProtocolManager::InternetControlMessageProtocolManager() : InternetProtocolHandler(InternetProtocolManager::instance(), 0x01) {}

        void InternetControlMessageProtocolManager::on_receive_ip(InternetProtocolV4Address sourceIPAddressBigEndian, InternetProtocolV4Address destinationIPAddressBigEndian, uint8_t* payload, uint32_t size)
        {
            if (size < sizeof(InternetControlMessageProtocolMessage)) { return; }

            auto message = (InternetControlMessageProtocolMessage*)payload;

            switch (message->get_type())
            {
            case 0:
                printf("PING response from");
                printf(sourceIPAddressBigEndian);
                printf("\n");
                break;
            
            case 8:
                message->set_type(0);
                message->set_checksum(0);
                message->set_checksum(InternetProtocolManager::instance()->checksum((uint16_t*)message, sizeof(InternetControlMessageProtocolMessage)));
                InternetProtocolManager::instance()->send_data(sourceIPAddressBigEndian, 0x01, (uint8_t*)message, sizeof(InternetControlMessageProtocolMessage));
            }
        }

        void InternetControlMessageProtocolManager::request_echo_reply(InternetProtocolV4Address IPAddressBigEndian)
        {
            InternetControlMessageProtocolMessage message;
            message.set_type(8);
            message.set_code(0);
            message.set_data(0x3713);
            message.set_checksum(0);
            message.set_checksum(InternetProtocolManager::instance()->checksum((uint16_t*)&message, sizeof(InternetControlMessageProtocolMessage)));

            InternetProtocolManager::instance()->send_data(IPAddressBigEndian, 0x01, (uint8_t*)&message, sizeof(InternetControlMessageProtocolMessage));
        }
    }
}
