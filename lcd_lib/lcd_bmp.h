/*
 * lcd_bmp.h
 *
 *  Created on: May 30, 2010
 *      Author: celebro
 */

#ifndef LCD_BMP_H_
#define LCD_BMP_H_

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

#endif /* LCD_BMP_H_ */
