/*
 * lcd_protocol.h
 *
 */

#ifndef LCD_PROTOCOL_H_
#define LCD_PROTOCOL_H_

/* Max string length for lcd_put_str */
#define LCD_MAX_STR_LEN 	25

/* Function calls */
#define LCD_SET_RECT	10
#define LCD_SET_LINE	11
#define LCD_PUT_STR		12
#define LCD_LOAD_BMP	13
#define LCD_SET_BMP		14

/* ioctl */
#define LCD_IOCTL_ON	1
#define LCD_IOCTL_OFF	0
#define LCD_IOCTL_ONOFF		1
#define LCD_IOCTL_SLEEP		2
#define LCD_IOCTL_BACKLIGHT	3

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
