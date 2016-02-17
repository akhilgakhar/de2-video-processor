/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include <io.h>
#include <system.h>
#include <sys/alt_stdio.h>
#include <string.h>

unsigned int palette_ega[16] =
		/* EGA Colour Palette */
		/* Black			  Blue	       Green       Cyan          Red */
		{/*00*/ 0x0000, /*01*/0x0015, /*02*/0x2704, /*03*/0x1E79, /*04*/0xA800,
		 /*     Magenta       Brown   Light Grey     Dark Grey   Bright Blue*/
		 /*05*/ 0xA815, /*06*/0xE3C1, /*07*/0xAD55, /*08*/0x52AA, /*09*/0x52BF,
		 /* BGreen            BCyan         Bred       B Magenta     B Yellow*/
		 /*10*/ 0x57EA, /*11*/0x57FF, /*12*/0xFAAA, /*13*/0xFABF, /*14*/0xFFEA,
		 0xFFFF /*B White */
		};

unsigned int palette_stephen[256] =
		/* Stephen's RGB323 -> RGB565 Palette */
		{0x0 , 0xa , 0x15 , 0x1f , 0x120 , 0x12a , 0x135 , 0x13f , 0x240 ,
		0x24a , 0x255 , 0x25f , 0x360 , 0x36a , 0x375 , 0x37f , 0x480 ,
		0x48a , 0x495 , 0x49f , 0x5a0 , 0x5aa , 0x5b5 , 0x5bf , 0x6c0 ,
		0x6ca , 0x6d5 , 0x6df , 0x7e0 , 0x7ea , 0x7f5 , 0x7ff , 0x2000 ,
		0x200a , 0x2015 , 0x201f , 0x2120 , 0x212a , 0x2135 , 0x213f , 0x2240 ,
		0x224a , 0x2255 , 0x225f , 0x2360 , 0x236a , 0x2375 , 0x237f , 0x2480 ,
		0x248a , 0x2495 , 0x249f , 0x25a0 , 0x25aa , 0x25b5 , 0x25bf , 0x26c0 ,
		0x26ca , 0x26d5 , 0x26df , 0x27e0 , 0x27ea , 0x27f5 , 0x27ff , 0x4800 ,
		0x480a , 0x4815 , 0x481f , 0x4920 , 0x492a , 0x4935 , 0x493f , 0x4a40 ,
		0x4a4a , 0x4a55 , 0x4a5f , 0x4b60 , 0x4b6a , 0x4b75 , 0x4b7f , 0x4c80 ,
		0x4c8a , 0x4c95 , 0x4c9f , 0x4da0 , 0x4daa , 0x4db5 , 0x4dbf , 0x4ec0 ,
		0x4eca , 0x4ed5 , 0x4edf , 0x4fe0 , 0x4fea , 0x4ff5 , 0x4fff , 0x6800 ,
		0x680a , 0x6815 , 0x681f , 0x6920 , 0x692a , 0x6935 , 0x693f , 0x6a40 ,
		0x6a4a , 0x6a55 , 0x6a5f , 0x6b60 , 0x6b6a , 0x6b75 , 0x6b7f , 0x6c80 ,
		0x6c8a , 0x6c95 , 0x6c9f , 0x6da0 , 0x6daa , 0x6db5 , 0x6dbf , 0x6ec0 ,
		0x6eca , 0x6ed5 , 0x6edf , 0x6fe0 , 0x6fea , 0x6ff5 , 0x6fff , 0x9000 ,
		0x900a , 0x9015 , 0x901f , 0x9120 , 0x912a , 0x9135 , 0x913f , 0x9240 ,
		0x924a , 0x9255 , 0x925f , 0x9360 , 0x936a , 0x9375 , 0x937f , 0x9480 ,
		0x948a , 0x9495 , 0x949f , 0x95a0 , 0x95aa , 0x95b5 , 0x95bf , 0x96c0 ,
		0x96ca , 0x96d5 , 0x96df , 0x97e0 , 0x97ea , 0x97f5 , 0x97ff , 0xb000 ,
		0xb00a , 0xb015 , 0xb01f , 0xb120 , 0xb12a , 0xb135 , 0xb13f , 0xb240 ,
		0xb24a , 0xb255 , 0xb25f , 0xb360 , 0xb36a , 0xb375 , 0xb37f , 0xb480 ,
		0xb48a , 0xb495 , 0xb49f , 0xb5a0 , 0xb5aa , 0xb5b5 , 0xb5bf , 0xb6c0 ,
		0xb6ca , 0xb6d5 , 0xb6df , 0xb7e0 , 0xb7ea , 0xb7f5 , 0xb7ff , 0xd800 ,
		0xd80a , 0xd815 , 0xd81f , 0xd920 , 0xd92a , 0xd935 , 0xd93f , 0xda40 ,
		0xda4a , 0xda55 , 0xda5f , 0xdb60 , 0xdb6a , 0xdb75 , 0xdb7f , 0xdc80 ,
		0xdc8a , 0xdc95 , 0xdc9f , 0xdda0 , 0xddaa , 0xddb5 , 0xddbf , 0xdec0 ,
		0xdeca , 0xded5 , 0xdedf , 0xdfe0 , 0xdfea , 0xdff5 , 0xdfff , 0xf800 ,
		0xf80a , 0xf815 , 0xf81f , 0xf920 , 0xf92a , 0xf935 , 0xf93f , 0xfa40 ,
		0xfa4a , 0xfa55 , 0xfa5f , 0xfb60 , 0xfb6a , 0xfb75 , 0xfb7f , 0xfc80 ,
		0xfc8a , 0xfc95 , 0xfc9f , 0xfda0 , 0xfdaa , 0xfdb5 , 0xfdbf , 0xfec0 ,
		0xfeca , 0xfed5 , 0xfedf , 0xffe0 , 0xffea , 0xfff5 , 0xffff };

unsigned int palette_magenta[256] = {0xA815};

unsigned int bunch_o_blues[256] = {0x000 , 0x001 , 0x002 , 0x003 , 0x004 , 0x005 , 0x006 , 0x007 , 0x008 ,
		0x009 , 0x00a , 0x00b , 0x00c , 0x00d , 0x00e , 0x00f , 0x0010 ,
		0x0011 , 0x0012 , 0x0013 , 0x0014 , 0x0015 , 0x0016 , 0x0017 , 0x0018 ,
		0x0019 , 0x001a , 0x001b , 0x001c , 0x001d , 0x001e , 0x001f };

unsigned int bunch_o_reds[256] = {0x000 , 0x010 , 0x020 , 0x030 , 0x040 , 0x050 , 0x060 , 0x070 , 0x080 ,
		0x090 , 0x0a0 , 0x0b0 , 0x0c0 , 0x0d0 , 0x0e0 , 0x0f0 , 0x0100 ,
		0x0110 , 0x0120 , 0x0130 , 0x0140 , 0x0150 , 0x0160 , 0x0170 , 0x0180 ,
		0x0190 , 0x01a0 , 0x01b0 , 0x01c0 , 0x01d0 , 0x01e0 , 0x01f0};



unsigned int* palettes[6] = {&palette_ega[0], &palette_stephen[0], &palette_magenta[0], &bunch_o_blues[0], &bunch_o_reds[0]};
unsigned int palettes_legnth = 5;



//Make a function to switch the palettes given a pointer to a palette array.
void switchPalette(unsigned int* palette, int length){

	int i = 0;
	for (i = 0; i < length; i++){
		IOWR_16DIRECT(COLOUR_PALETTE_SHIFTER_0_BASE, 2*i, 0x0000);
		IOWR_16DIRECT(COLOUR_PALETTE_SHIFTER_0_BASE, 2*i, palette[i]);
	}


}


void printPalette(int n){
	// Print everything in the palette ram, upto int colours.
	int i;
	unsigned int c;

	unsigned int results[512] = {'\0'};

	for (i = 0; i < n; i++){
		c = IORD_16DIRECT(COLOUR_PALETTE_SHIFTER_0_BASE, 2*i); //offset multiplied by 2 to be on 16-bit boundaries.
		//alt_printf("palette[ %x ]: %x ", 2*i, c);
		results[i] = c;
	}

	for (i = 0; i < n; i++){
		alt_printf("palette[ %x ]: %x ", 2*i, results[i]);
	}

}

int main()
{


	unsigned int row = 0;
	unsigned int col = 0;
	unsigned int delay = 0;

	unsigned int color;
	int i;



	// Clear the screen first.
	alt_putstr("Clear the screen\n");
	for (col = 0; col < 640; col = col + 4){
		for (row = 0; row < 480; row++){
			color = 0;
			IOWR_32DIRECT(SRAM_0_BASE, row * 640 + col, color << 24 | color << 16 | color << 8 | color << 0);
		}
	}


	//Switch palettes here (to EGA colour palette)
	printPalette(16);
	switchPalette(palette_ega, 16);
	printPalette(16);

	alt_putstr("Screen painting demo\n");

	for (i = 0; i < 16; i++){
		alt_printf("Colour");
		for (delay = 0; delay < 700/*2000*/; delay++){
			unsigned int tdelay = delay;
			for (tdelay; tdelay > 0; tdelay--){}
		}

		for (row=0; row<480; row++){
			for (col = 0; col < 640; col=col+4){
				color = i;
				IOWR_32DIRECT(SRAM_0_BASE, row * 640 + col, color << 24 | color << 16 | color << 8 | color << 0);
			}

		}
	}



	alt_putstr("\nStarting Stephen Test Pattern\n");
	int p = 0;
	for (p = 0; p < 8; p++) {

		switchPalette(palettes[p%palettes_legnth], 256);

		alt_putstr("Clear the screen\n");
		for (col = 0; col < 640; col = col + 4){
			for (row = 0; row < 480; row++){
				color = 0;
				IOWR_32DIRECT(SRAM_0_BASE, row * 640 + col, color << 24 | color << 16 | color << 8 | color << 0);
			}
		}


		//Now draw the test pattern
		for (row = 0; row < 480; row++)
		{
			for (col = 0; col < 640; col = col + 4)
			{
				color = ((row + col) % 256) << 0 | ((row + col) % 256) << 8 | ((row + col) % 256) << 16 | ((row + col) % 256) << 24;
				if (row == 0 || row == 479)
				{
					IOWR_32DIRECT(SRAM_0_BASE, row * 640 + col, 0xFFFFFFFF);
				}
				else if (col == 0)
				{
					IOWR_32DIRECT(SRAM_0_BASE, row * 640 + col, 0x000000FF | color);
				}
				else if (col == 636)
				{
					IOWR_32DIRECT(SRAM_0_BASE, row * 640 + col, 0xFF000000 | color);
				}
				else
				{
					IOWR_32DIRECT(SRAM_0_BASE, row * 640 + col, color);
				}
			}
		}


	}





	alt_putstr("Done.\n");


	// Do Stephen's pattern display.

	// And switch the palettes.


  return 0;
}