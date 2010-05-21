/*
 * display_lib.h
 *
 *  Created on: 20.5.2010
 *      Author: Jure
 */

#ifndef DISPLAY_LIB_H_
#define DISPLAY_LIB_H_

#define TX_MAX 10000

struct display {
	int file;
	unsigned int tx_index;
	unsigned char tx_data[TX_MAX];
	unsigned int lcd_image[130][130];
};

struct display* get_display();
void release_display(struct display* display);

void WriteSpiCommand(volatile unsigned char command, struct display* display);
void WriteSpiData(volatile unsigned char command, struct display* display);
void LCDSetXY(int x, int y, struct display* display);
void LCDSetPixel(int x, int y, int color, struct display* display);
void LCDSetLine(int x1, int y1, int x2, int y2, int color, struct display* display);
void LCDSetRectangle(int x0, int y0, int x1, int y1, unsigned char fill, int color, struct display* display);

#endif /* DISPLAY_LIB_H_ */
