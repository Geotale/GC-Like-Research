#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#ifdef GAMECUBE
	#include <gccore.h>
#else
	#include <wiiuse/wpad.h>
#endif

// Put after the previous includes to avoid warnings
#include "exi.h"
#include "sram.h"
#include "test.h"


// For the framebuffer
static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

int main(int argc, char **argv) {
	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
#ifdef GAMECUBE
	PAD_Init();
#else
	WPAD_Init();
#endif

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);

	// Make the display visible
	VIDEO_SetBlack(false);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if( rmode->viTVMode & VI_NON_INTERLACE ) VIDEO_WaitVSync();

	// Read from SRAM
	#define SIZE (64)
	uint32_t cmd = SRAM_ADDR << DEV_CMD_BITS;
	// Create an address to store the destination
	uint8_t result[SIZE] __attribute__ ((aligned (32)));

	for (uint32_t i = 0; i < SIZE; i++) {
		// Have the default values be simply counting down
		result[i] = SIZE - i;
	}

	// Select the channel
	_EXI_Select(EXI_CHANNEL_0, EXI_DEVICE_1, EXI_SPEED8MHZ);
	// Send the command over
	_EXI_Imm(EXI_CHANNEL_0, cmd, 4, EXI_WRITE);
	// Print the command value that was written back
	printf("%08x\n", *(__channel_addr(EXI_CHANNEL_0) + 4));

	for (uint32_t i = 0; i < SIZE; i++) {
		// Read back one byte at a time from SRAM
		result[i] = _EXI_Imm(EXI_CHANNEL_0, 0, 1, EXI_READ);
	}

	// Print the final value in the immediate transfer buffer
	printf("%08x\n", *(__channel_addr(EXI_CHANNEL_0) + 4));

	display_result(result, SIZE);

	// Wait in an idle loop before exiting
	while (1) {
		#ifdef GAMECUBE
			PAD_ScanPads();

			u32 pressed = PAD_ButtonsDown(0);
			if ( pressed & PAD_BUTTON_START ) exit(0);
		#else
			WPAD_ScanPads();

			u32 pressed = WPAD_ButtonsDown(0);
			if ( pressed & WPAD_BUTTON_HOME ) exit(0);
		#endif

		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}