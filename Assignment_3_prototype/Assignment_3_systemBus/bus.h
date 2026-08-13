#ifndef BUS_H
#define BUS_H

//#include <stdint.h>

#define BUS_INSTR_BASE 0x0000
#define BUS_INSTR_SIZE 256
#define BUS_DATA_BASE  0x0100
#define BUS_DATA_SIZE  256

// typedef unsigned char (*bus_read_fn)(unsigned int offset);
// typedef void (*bus_write_fn)(unsigned int offset, unsigned char value);

// /* Registers a memory-mapped device on the bus. base/size define the
//  * region of the unified address space the device owns; read_fn/write_fn
//  * are always called with an offset relative to base (i.e. 0-based). */
// void bus_register_device(const char *name, unsigned int base, unsigned int size,
//                           bus_read_fn read_fn, bus_write_fn write_fn);

// /* Single-byte transactions against the unified bus address space.
//  * An access to an address no device owns is reported to stderr and
//  * treated as a read of 0 / a discarded write, rather than crashing. */
// unsigned char bus_read8(unsigned int address);
// void bus_write8(unsigned int address, unsigned char value);


void bus_init(void);
unsigned char bus_read8(unsigned int addr);
void bus_write8(unsigned int addr, unsigned char val);

// unsigned char bus_read32(unsigned char addr);
// void bus_write32(unsigned char addr, unsigned char val);

#endif
