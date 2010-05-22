/*
 * display_lib.h
 *
 *  Created on: 20.5.2010
 *      Author: Jure
 */

#ifndef DISPLAY_LIB_H_
#define DISPLAY_LIB_H_

#define TX_MAX 256

struct display {
	int file;
	unsigned int tx_index;
	unsigned char tx_data[TX_MAX];
};

struct display* get_display();
void release_display(struct display* display);

void write_to_file(struct display *display);

void lcd_set_pixel(int x, int y, int color, struct display* display);
void lcd_set_rect(int x1, int y1, int x2, int y2, unsigned char fill, int color, struct display*);
void lcd_set_line(int x1, int y1, int x2, int y2, int color, struct display*);
void lcd_put_str(char *str, int x, int y, int size, int color, struct display *display);



#endif /* DISPLAY_LIB_H_ */
