#include <stdio.h>
#include <stdint.h>

// To avoid overwriting
#ifndef EXI_CHANNEL_0

#define EXI_CHANNEL_0 0
#define EXI_CHANNEL_1 1
#define EXI_CHANNEL_2 2

#define EXI_DEVICE_0 0b001
#define EXI_DEVICE_1 0b010
#define EXI_DEVICE_2 0b100

#define EXI_READ      0
#define EXI_WRITE     1
#define EXI_READWRITE 2

#define EXI_SPEED1MHZ  0
#define EXI_SPEED2MHZ  1
#define EXI_SPEED4MHZ  2
#define EXI_SPEED8MHZ  3
#define EXI_SPEED16MHZ 4
#define EXI_SPEED32MHZ 5

#endif

#ifdef GAMECUBE
	#define IO 0xcc000000
#else
	#define IO 0xcd000000
#endif

volatile uint32_t* __channel_addr(uint32_t channel) {
	return (volatile uint32_t *)(IO | 0x6800) + 5 * channel;
}

void _EXI_Sync(uint32_t channel) {
	volatile uint32_t* control_addr = __channel_addr(channel) + 3;

	while (*control_addr & 1) {
		// Do nothing
	}
}

void _EXI_Select(uint32_t channel, uint32_t device, uint32_t speed) {
	_EXI_Sync(channel);

	volatile uint32_t* status_addr = __channel_addr(channel) + 0;
	*status_addr = ((1 << device) << 7) | (speed << 4) | 0b100000001010;
}

void _EXI_Deselect(uint32_t channel) {
	_EXI_Sync(channel);

	volatile uint32_t* status_addr = __channel_addr(channel) + 0;
	*status_addr = 0b100000001010;
}

uint32_t _EXI_Imm(uint32_t channel, uint32_t data, uint32_t length, uint32_t type) {
	_EXI_Sync(channel);

	volatile uint32_t* control_addr = __channel_addr(channel) + 3;
	volatile uint32_t* imm_addr = __channel_addr(channel) + 4;

	if (type == EXI_WRITE) {
		uint32_t value = *imm_addr;

		for (uint32_t i = 0; i < length; i++) {
			uint32_t mask = 0xff000000 >> (8 * i);
			value &= ~mask;
			value |= data & mask;
		}

		// printfs are for debugging purposes
		printf("%08x | %08x\n", data, length);
		*imm_addr &= (1 << (8 * (4 - length))) - 1;
		*imm_addr |= data << (8 * (4 - length));
		printf("%08x\n", *imm_addr);
	}

	*control_addr = (length - 1) << 4 | type << 2 | 1;

	_EXI_Sync(channel);

	return *imm_addr >> (8 * (4 - length));
}

// So even reads end up writing to the immediate address
uint32_t _EXI_Imm_force(uint32_t channel, uint32_t data, uint32_t length, uint32_t type) {
	_EXI_Sync(channel);

	volatile uint32_t* control_addr = __channel_addr(channel) + 3;
	volatile uint32_t* imm_addr = __channel_addr(channel) + 4;

	uint32_t value = *imm_addr;

	for (uint32_t i = 0; i < length; i++) {
		uint32_t mask = 0xff000000 >> (8 * i);
		value &= ~mask;
		value |= data & mask;
	}

	printf("%08x | %08x (FORCE)\n", data, length);
	*imm_addr &= (1 << (8 * (4 - length))) - 1;
	*imm_addr |= data << (8 * (4 - length));
	printf("%08x\n", *imm_addr);

	*control_addr = (length - 1) << 4 | type << 2 | 1;

	_EXI_Sync(channel);

	return *imm_addr >> (8 * (4 - length));
}

uint32_t _EXI_Dma(uint32_t channel, void* data, uint32_t length, uint32_t type) {
	_EXI_Sync(channel);

	volatile void** addr_addr = (volatile void**)(__channel_addr(channel) + 1);
	volatile uint32_t* length_addr = __channel_addr(channel) + 2;
	volatile uint32_t* control_addr = __channel_addr(channel) + 3;
	volatile uint32_t* imm_addr = __channel_addr(channel) + 4;

	if (((uint32_t)data & 0x1f) != 0) {
		printf("Invalid DMA address! Needs to be 32-bit aligned!\n");
	}
	if ((length & 0x1f) != 0) {
		printf("Invalid DMA length! Needs to be a multiple of 32!\n");
	}

	*addr_addr = data;
	*length_addr = length;
	*control_addr = type << 2 | 0b11;

	_EXI_Sync(channel);

	return *imm_addr;
}