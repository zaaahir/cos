#ifndef NETWORKDRIVER_H
#define NETWORKDRIVER_H

#include "types.h"
#include "common/doublyLinkedList.h"

namespace Drivers
{
    class EthernetNetworkDriverEventHandler;
    class EthernetNetworkDriver
    {
    friend class EthernetNetworkDriverEventHandler;
    public:
        virtual uint64_t get_MAC_address() = 0;
        virtual uint32_t get_IP_address() = 0;
        virtual void set_IP_address(uint32_t IPAddress) = 0;
        virtual void send_data(uint8_t* buf, uint64_t size) = 0;
    protected:
        Common::DoublyLinkedList<EthernetNetworkDriverEventHandler*> m_handlers;
        void add_handler(EthernetNetworkDriverEventHandler* handler);
        void remove_handler(EthernetNetworkDriverEventHandler* handler);
    };

    class EthernetNetworkDriverEventHandler
    {
    public:
        virtual void on_receive_data(uint8_t* buf, uint64_t size) = 0;
        EthernetNetworkDriverEventHandler(EthernetNetworkDriver* driver);
        ~EthernetNetworkDriverEventHandler();
    protected:
        EthernetNetworkDriver* m_driver;
    };
}

#endif
