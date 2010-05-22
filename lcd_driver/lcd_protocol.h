/*
 * lcd_protocol.h
 *
 */

#ifndef LCD_PROTOCOL_H_
#define LCD_PROTOCOL_H_

#define LCD_SET_RECT	10
#define LCD_SET_LINE	11

struct lcd_func_params {
	int x1;
	int y1;
	int x2;
	int y2;
	int fill;
	int color;
};


#endif /* LCD_PROTOCOL_H_ */
