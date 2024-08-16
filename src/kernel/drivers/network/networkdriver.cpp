#include "drivers/network/networkdriver.h"

namespace Drivers
{
    void EthernetNetworkDriver::add_handler(EthernetNetworkDriverEventHandler* handler)
    {
        m_handlers.append(handler);
    }

    void EthernetNetworkDriver::remove_handler(EthernetNetworkDriverEventHandler* handler)
    {
        auto it = m_handlers.first();
        for (; !it.is_end(); ++it) {
            if (*it == handler) { break; }
        }
        if (*it == handler) { m_handlers.remove(it); };
    }

    EthernetNetworkDriverEventHandler::EthernetNetworkDriverEventHandler(EthernetNetworkDriver* driver) : m_driver(driver)
    {
        m_driver->add_handler(this);
    }

    EthernetNetworkDriverEventHandler::~EthernetNetworkDriverEventHandler() { m_driver->remove_handler(this); }

}
