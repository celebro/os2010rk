/*
 * display_lib.h
 *
 *  Created on: 20.5.2010
 *      Author: Jure
 */

#ifndef DISPLAY_LIB_H_
#define DISPLAY_LIB_H_

// 12-bit color definitions
#define WHITE          0xFFF
#define BLACK          0x000
#define RED            0xF00
#define GREEN          0x0F0
#define BLUE           0x00F
#define CYAN           0x0FF
#define MAGENTA        0xF0F
#define YELLOW         0xFF0
#define BROWN          0xB22
#define ORANGE         0xFA0
#define PINK           0xF6A

// Font sizes
#define SMALL          0
#define MEDIUM         1
#define LARGE          2

// Filled rectangle
#define FILL	1
#define NOFILL	0

// ON OFF parameters
#define LCD_ON	1
#define LCD_OFF	0

// Max transfer to device file
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
void lcd_load_bmp(char *file, struct display *display);
void lcd_set_bmp(int x, int y, struct display *display);

/* Ioctl */
void lcd_sleep(int state, struct display *display);
void lcd_backlight(int state, struct display *display);
void lcd_onoff(int state,  struct display *display);

#endif /* DISPLAY_LIB_H_ */
