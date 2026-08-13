#include <stdio.h>
#include "bus.h"
#include "memory.h"

// #define MAX_DEVICES 8

// typedef struct {
//     const char *name;
//     unsigned int base;
//     unsigned int size;
//     bus_read_fn read_fn;
//     bus_write_fn write_fn;
// } BusDevice;

// static BusDevice devices[MAX_DEVICES];
// static int deviceCount = 0;

// void bus_register_device(const char *name, unsigned int base, unsigned int size,
//                           bus_read_fn read_fn, bus_write_fn write_fn) {
//     if (deviceCount >= MAX_DEVICES) {
//         fprintf(stderr, "Bus error: no room left to register device '%s'\n", name);
//         return;
//     }
//     devices[deviceCount].name = name;
//     devices[deviceCount].base = base;
//     devices[deviceCount].size = size;
//     devices[deviceCount].read_fn = read_fn;
//     devices[deviceCount].write_fn = write_fn;
//     deviceCount++;
// }

// /* Address decoding: finds which registered device (if any) owns this
//  * unified-bus address, and returns the offset local to that device. */
// static BusDevice *decodeAddress(unsigned int address, unsigned int *offset) {
//     for (int i = 0; i < deviceCount; i++) {
//         if (address >= devices[i].base && address < devices[i].base + devices[i].size) {
//             *offset = address - devices[i].base;
//             return &devices[i];
//         }
//     }
//     return NULL;
// }

// unsigned char bus_read8(unsigned int address) {
//     unsigned int offset;
//     BusDevice *dev = decodeAddress(address, &offset);
//     if (!dev) {
//         fprintf(stderr, "Bus error: read from unmapped address 0x%04X\n", address);
//         return 0;
//     }
//     return dev->read_fn(offset);
// }

// void bus_write8(unsigned int address, unsigned char value) {
//     unsigned int offset;
//     BusDevice *dev = decodeAddress(address, &offset);
//     if (!dev) {
//         fprintf(stderr, "Bus error: write to unmapped address 0x%04X\n", address);
//         return;
//     }
//     dev->write_fn(offset, value);
// }

void bus_init(void) {
    initialize(); 
}

unsigned char bus_read8(unsigned int addr) {
    
    // 1. RAM Access
    if (addr >= BUS_DATA_BASE && addr < (BUS_DATA_BASE + BUS_DATA_SIZE)) {
        return (unsigned char)Data[addr - BUS_DATA_SIZE];
    }
    // 2. ROM (Instruction Memory) Access
    if (addr >= BUS_INSTR_BASE && addr < (BUS_INSTR_BASE + BUS_INSTR_SIZE)) {
        return (unsigned char)Instruction[addr - BUS_INSTR_BASE];
    }

    printf("BUS ERROR : Address is out of bounds");
    return 0;
}

void bus_write8(unsigned int addr, unsigned char val) {

    // 1. RAM Access
    if (addr >= BUS_DATA_BASE && addr < (BUS_DATA_BASE + BUS_DATA_SIZE)) {
        Data[addr - BUS_DATA_BASE] = (char)val;
        return;
    }
    // 2. ROM Protection
    if (addr >= BUS_INSTR_BASE && addr < (BUS_INSTR_BASE + BUS_INSTR_SIZE)) {
        printf("[BUS ERROR] Attempted write to Read-Only ROM\n");
        return;
    }

    printf("BUS ERROR : Address is out of bounds");
}
