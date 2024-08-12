#ifndef PCI_H
#define PCI_H 
#include "types.h"
#include "io.h"
#include "common/doublyLinkedList.h"

namespace PeripheralComponentInterconnect {

    #define CONFIG_ADDRESS 0xCF8
    #define CONFIG_DATA 0xCFC
    #define IO_SPACE_BAR 0x1

    struct PCIDevice {
        uint8_t bus;
        uint8_t device;
        uint8_t function;
        uint16_t deviceId;
        uint16_t vendorId;
        uint8_t classCode;
        uint8_t subclassCode;
        uint8_t revision;
        uint8_t interruptLine;
        uint32_t ioPortBase;
    };

    class PCIManager{
    public:
        PCIManager();
        ~PCIManager() = default;
        uint32_t read_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
        void write_dword(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);
        PCIDevice* get_device(uint16_t vendorId, uint16_t deviceId);
        Common::DoublyLinkedList<PCIDevice> m_devices;
        static PCIManager* instance();
    private:
        static PCIManager* m_self;
        static constexpr uint16_t CONFIG_ADDRESS_PORT = 0xCF8;
        static constexpr uint16_t CONFIG_DATA_PORT = 0xCFC;
        void find_devices();
        PCIDevice get_pci_device(uint8_t bus, uint8_t device, uint8_t function);
        uint8_t device_functions_count(uint8_t bus, uint8_t device);
    };
}

#endif
