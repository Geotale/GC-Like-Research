#include <stdio.h>
#include <stdint.h>

// Print a simple hexadecimal view of the result
// There's a warning about this but it's nicer than having to create a new file
void display_result(uint8_t result[], uint32_t size) { // NOLINT
	for (uint32_t i = 0; i < size; i += 16) {
		for (uint32_t j = 0; j < 16; j++) {
			printf("%02x ", result[i + j]);
		}
		printf(" |  ");
		for (uint32_t j = 0; j < 16; j++) {
			if (result[i + j] >= 0x20 && result[i + j] <= 0x7e) {
				printf("%c", (char)result[i + j]);
			} else {
				printf(".");
			}
		}
		printf("\n");
	}
}