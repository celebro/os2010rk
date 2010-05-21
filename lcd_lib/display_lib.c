/*
 * display_lib.c
 *
 *  Created on: 7.5.2010
 *      Author: Jure
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "display_lib.h"
#include "lcd.h"

unsigned char array[2];

struct display* get_display() {
	/* Stucture to lcd dispaly, so there is nothing global */
	struct display* display = malloc(sizeof(struct display));
	if (display == NULL) {
		printf("Failed to allocate memmory for device struct\n");
		return NULL;
	}

	display->file = open("/dev/lcd", O_WRONLY);
	if (display->file < 0) {
		printf("Failed to open device /dev/lcd\n");
		release_display(display);
		return NULL;
	}

	display->tx_index = 0;

	// TODO memory image

	return display;
}

void release_display(struct display* display) {
	if (display) {
		close(display->file);
		free(display);
	}
}

void WriteToFile(struct display* display) {
	if (display == NULL) {
		printf("ERROR: Null display pointer-> WriteInFile\n");
		return;
	}

	if (display->tx_index < 2) {
		printf("ERROR: Writing half command-> WriteInFile\n");
		return;
	}

	write(display->file, display->tx_data, display->tx_index+1);
	display->tx_index = 0;
}

inline void WriteSpiCommand(volatile unsigned char command, struct display* display){
	display->tx_data[display->tx_index++] = 0;
	display->tx_data[display->tx_index++] = command;
}

inline void WriteSpiData(volatile unsigned char data, struct display* display){
	display->tx_data[display->tx_index++] = 1;
	display->tx_data[display->tx_index++] = data;
}

void LCDSetXY(int x, int y, struct display* display){
	WriteSpiCommand(PASET, display);	// Row address set
	WriteSpiData(x, display);//start
	WriteSpiData(x, display);//finish

	WriteSpiCommand(CASET, display);// Column address set
	WriteSpiData(y, display);//start
	WriteSpiData(y, display);//finish
}

void LCDSetPixel(int x, int y, int color, struct display* display){
	LCDSetXY(x, y, display);
	WriteSpiCommand(RAMWR, display);// memory write
	WriteSpiData((unsigned char)((color >> 4) & 0xFFFF), display);//8 bits of color are sent
	WriteSpiData((unsigned char)(((color & 0x0F) << 4) | 0x00), display);//the remaining 4 are sent
	WriteSpiCommand(NOP, display);
}

void LCDSetRectangle(int x0, int y0, int x1, int y1, int color, struct display* display) {
	int xmin, xmax, ymin, ymax;
	int i;
	// best way to create a filled rectangle is to define a drawing box
	// and loop two pixels at a time
	// calculate the min and max for x and y directions
	xmin = (x0 <= x1) ? x0 : x1;
	xmax = (x0 > x1) ? x0 : x1;
	ymin = (y0 <= y1) ? y0 : y1;
	ymax = (y0 > y1) ? y0 : y1;
	// specify the controller drawing box according to those limits
	WriteSpiCommand(PASET, display);// Row address set
	WriteSpiData(xmin, display);
	WriteSpiData(xmax, display);

	WriteSpiCommand(CASET, display);// Column address set
	WriteSpiData(ymin, display);
	WriteSpiData(ymax, display);

	WriteSpiCommand(RAMWR, display);// WRITE MEMORY
	// loop on total number of pixels / 2
	// use the color value to output three data bytes covering two pixels
	// if number of pixels is odd we lose one pixel(rounding error)-> we add +1 in formula
	for (i = 0; i < ((((xmax - xmin + 1) * (ymax - ymin + 1)) / 2) + 1); i++) {
		WriteSpiData((unsigned char)((color >> 4) & 0xFF), display);
		WriteSpiData((unsigned char)(((color & 0xF) << 4) | ((color >> 8) & 0xF)), display);
		WriteSpiData((unsigned char)(color & 0xFF), display);
	}
}

