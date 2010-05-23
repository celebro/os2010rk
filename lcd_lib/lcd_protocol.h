/*
 * lcd_protocol.h
 *
 */

#ifndef LCD_PROTOCOL_H_
#define LCD_PROTOCOL_H_

#define LCD_MAX_STR_LEN 	25

#define LCD_SET_RECT	10
#define LCD_SET_LINE	11
#define LCD_PUT_STR		12
#define LCD_LOAD_BMP	13
#define LCD_SET_BMP		14

struct lcd_func_params {
	int x1;
	int y1;
	int x2;
	int y2;
	int fill;
	int color;
	int t_len;
	int t_size;
	int *bmp;
};


#endif /* LCD_PROTOCOL_H_ */
