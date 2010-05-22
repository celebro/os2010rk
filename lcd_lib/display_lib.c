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
#include "lcd_protocol.h"

struct display* get_display() {
	/* Stucture to lcd display, so there is nothing global */
	struct display* display = malloc(sizeof(struct display));
	if (display == NULL) {
		printf("Failed to allocate memmory for device struct\n");
		return NULL;
	}

	/* File pointer to character device > DRIVER */
	display->file = open("/dev/lcd", O_WRONLY);
	if (display->file < 0) {
		printf("Failed to open device /dev/lcd\n");
		release_display(display);
		return NULL;
	}

	display->tx_index = 0;

	lcd_set_rect(0,0, 130, 130, FILL, BLACK, display);

	return display;
}

/* Delete display structure */
void release_display(struct display* display) {
	if (display) {
		close(display->file);
		free(display);
	}
}

void write_to_file(struct display* display) {
	int i;

	if (display == NULL) {
		printf("ERROR: Null display pointer-> WriteInFile\n");
		return;
	}

//	for (i = 0; i < display->tx_index; i++) {
//		printf("%d\n", display->tx_data[i]);
//	}
//	printf("\n");

	/* Send local buffer to char device */
	write(display->file, display->tx_data, display->tx_index);
	/* Set buffer empty */
	display->tx_index = 0;

//	/* Need 9 bits per command, don't send only one byte*/
//	if (display->tx_index < 2) {
//		printf("ERROR: Writing half command-> WriteInFile\n");
//		return;
//	}

}

void lcd_set_pixel(int x, int y, int color, struct display* display) {
	lcd_set_rect(x, y, x, y, FILL, color, display);
}

void lcd_set_rect(int x1, int y1, int x2, int y2, unsigned char fill, int color, struct display* display) {
	struct lcd_func_params param;
	param.x1 = x1;
	param.y1 = y1;
	param.x2 = x2;
	param.y2 = y2;
	param.fill = fill;
	param.color = color;

	/* Set data type */
	display->tx_data[display->tx_index++] = LCD_SET_RECT;
	/* Copy parameters */
	memcpy(&(display->tx_data[display->tx_index]), &param, sizeof(struct lcd_func_params));
	display->tx_index += sizeof(struct lcd_func_params);

	write_to_file(display);
}

void lcd_set_line(int x1, int y1, int x2, int y2, int color, struct display* display) {
	struct lcd_func_params param;
	param.x1 = x1;
	param.y1 = y1;
	param.x2 = x2;
	param.y2 = y2;
	param.color = color;

	/* Set data type */
	display->tx_data[display->tx_index++] = LCD_SET_LINE;

	/* Copy parameters */
	memcpy(&(display->tx_data[display->tx_index]), &param, sizeof(struct lcd_func_params));
	display->tx_index += sizeof(struct lcd_func_params);

	write_to_file(display);
}

void lcd_put_str(char *str, int x, int y, int size, int color, struct display *display) {
	struct lcd_func_params param;
	int length;

	param.x1 = x;
	param.y1 = y;
	param.color = color;

	/* Max LCD_MAX_STR_LEN characters */
	length = strlen(str);

	if (length > LCD_MAX_STR_LEN)
		length = LCD_MAX_STR_LEN;

	param.t_len = length;
	param.t_size = size;

	/* Set data type */
	display->tx_data[display->tx_index++] = LCD_PUT_STR;

	/* Copy parameters */
	memcpy(&(display->tx_data[display->tx_index]), &param, sizeof(struct lcd_func_params));
	display->tx_index += sizeof(struct lcd_func_params);

	/* Copy string */
	memcpy(&(display->tx_data[display->tx_index]), str, length);
	display->tx_index += length+1;
	display->tx_data[display->tx_index] = '\n';

	write_to_file(display);
}


