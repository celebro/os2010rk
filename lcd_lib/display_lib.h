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

//typedef struct tagBITMAPFILEHEADER
//{
//	WORD bfType;  //specifies the file type
//	DWORD bfSize;  //specifies the size in bytes of the bitmap file
//	WORD bfReserved1;  //reserved; must be 0
//	WORD bfReserved2;  //reserved; must be 0
//	DWORD bOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
//	  uint32_t filesz;
//	  uint16_t creator1;
//	  uint16_t creator2;
//	  uint32_t bmp_offset;
//
//}BITMAPFILEHEADER;
//
//typedef struct tagBITMAPINFOHEADER
//{
//	DWORD biSize;  //specifies the number of bytes required by the struct
//	LONG biWidth;  //specifies width in pixels
//	LONG biHeight;  //species height in pixels
//	WORD biPlanes; //specifies the number of color planes, must be 1
//	WORD biBitCount; //specifies the number of bit per pixel
//	DWORD biCompression;//spcifies the type of compression
//	DWORD biSizeImage;  //size of image in bytes
//	LONG biXPelsPerMeter;  //number of pixels per meter in x axis
//	LONG biYPelsPerMeter;  //number of pixels per meter in y axis
//	DWORD biClrUsed;  //number of colors used by th ebitmap
//	DWORD biClrImportant;  //number of colors that are important
//}BITMAPINFOHEADER;


struct display* get_display();
void release_display(struct display* display);

void write_to_file(struct display *display);

void lcd_set_pixel(int x, int y, int color, struct display* display);
void lcd_set_rect(int x1, int y1, int x2, int y2, unsigned char fill, int color, struct display*);
void lcd_set_line(int x1, int y1, int x2, int y2, int color, struct display*);
void lcd_put_str(char *str, int x, int y, int size, int color, struct display *display);
void lcd_load_bmp(char *file, struct display *display);
void lcd_set_bmp(int x, int y, struct display *display);



#endif /* DISPLAY_LIB_H_ */
