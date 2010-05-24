/*
 * display_lib.h
 *
 *  Created on: 20.5.2010
 *      Author: Jure
 */

#ifndef DISPLAY_LIB_H_
#define DISPLAY_LIB_H_

#include <stdint.h>

#define TX_MAX 256

#define LCD_ON	1
#define LCD_OFF	0

struct display {
	int file;
	unsigned int tx_index;
	unsigned char tx_data[TX_MAX];
};

struct bmpfile_magic {
  unsigned char magic[2];
};

struct bmpfile_header {
  uint32_t filesz;
  uint16_t creator1;
  uint16_t creator2;
  uint32_t bmp_offset;
};

struct bmpfile_info{
  uint32_t header_sz;
  uint32_t width;
  uint32_t height;
  uint16_t nplanes;
  uint16_t bitspp;
  uint32_t compress_type;
  uint32_t bmp_bytesz;
  uint32_t hres;
  uint32_t vres;
  uint32_t ncolors;
  uint32_t nimpcolors;
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
