/*
 * display_lib.c
 *
 *  Created on: 7.5.2010
 *      Author: Jure
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

	lcd_set_rect(0,0, 129, 129, FILL, BLACK, display);

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

	/* Send local buffer to char device */
	write(display->file, display->tx_data, display->tx_index);
	/* Set buffer empty */
	display->tx_index = 0;
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


void lcd_load_bmp(char *file_name, struct display* display) {
	FILE *file = NULL; //our file pointer

	struct bmpfile_magic	bmp_magic;
	struct bmpfile_header	bmp_header;
	struct bmpfile_info		bmp_info;

	unsigned char *bitmap_image = NULL;
	unsigned int *bmp = NULL;
	int i = 0,j;
	int x, y;
	unsigned int tempR, tempG, tempB;
	struct lcd_func_params param;


	//open filename in read binary mode
	file = fopen(file_name, "rb");
	if (file == NULL) {
		printf("Error: Failed to open bitmap file\n");
		return;
	}

	/* Read magic number */
	fread(&bmp_magic, sizeof(struct bmpfile_magic), 1, file);
	/* Read the bitmap file header */
	fread(&bmp_header, sizeof(struct bmpfile_header), 1, file);

	/* Verify magic number */
	if ((bmp_magic.magic[0] !=0x42) || (bmp_magic.magic[1] != 0x4D)) {
		printf("Error: Invalid bitmap: %x,%x\n",bmp_magic.magic[0],bmp_magic.magic[1]);
		fclose(file);
		return;
	}

	/* Read the bitmap info header */
	fread(&bmp_info, sizeof(struct bmpfile_info), 1, file);

	if (bmp_info.bitspp != 0x18) {
		printf("Error: Can only read 24bit bmp\n");
		free(bitmap_image);
		fclose(file);
		return;
	}

	/* Move to begging of bitmap data */
	fseek(file, bmp_header.bmp_offset, SEEK_SET);

	//allocate enough memory for the bitmap image data
	bitmap_image = (unsigned char*) malloc(bmp_info.bmp_bytesz);

	if (!bitmap_image)
	{
		printf("Error: Failed to allocate memmory for bitmap\n");
		free(bitmap_image);
		fclose(file);
		return;
	}
	/* Read in the bitmap data */
	fread(bitmap_image, bmp_info.bmp_bytesz, 1, file);

	/* Bmp in 12 bits for lcd */
	bmp = (unsigned int*) malloc(bmp_info.height*bmp_info.width*sizeof(unsigned int));
	if (!bmp)
	{
		printf("Error: Failed to allocate memmory for new bmp\n");
		free(bitmap_image);
		fclose(file);
		return;
	}

	i = 0;
	j = 0;

	for (x = 0; x < bmp_info.height; x++) {
		for (y = 0; y < bmp_info.width; y++) {
			tempB = bitmap_image[i++] >> 4;
			tempG = bitmap_image[i++] >> 4;
			tempR = bitmap_image[i++] >> 4;

			bmp[j++] = ((tempR << 8) & 0x0F00) | ((tempG << 4) & 0x00F0) | ((tempB) & 0x000F);
		}

		i += bmp_info.width % 4;
	}

	param.x1 = bmp_info.height;
	param.y1 = bmp_info.width;
	param.bmp = bmp;

	/* Set data type */
	display->tx_data[display->tx_index++] = LCD_LOAD_BMP;
	/* Copy parameters */
	memcpy(&(display->tx_data[display->tx_index]), &param, sizeof(struct lcd_func_params));
	display->tx_index += sizeof(struct lcd_func_params);

	write_to_file(display);

	free(bmp);
	free(bitmap_image);
}

void lcd_set_bmp(int x, int y, struct display * display) {
	struct lcd_func_params param;
	param.x1 = x;
	param.y1 = y;

	/* Set data type */
	display->tx_data[display->tx_index++] = LCD_SET_BMP;

	/* Copy parameters */
	memcpy(&(display->tx_data[display->tx_index]), &param, sizeof(struct lcd_func_params));
	display->tx_index += sizeof(struct lcd_func_params);

	write_to_file(display);
}

void lcd_sleep(int state, struct display *display) {
	if (state == LCD_ON) {
		ioctl(display->file, LCD_IOCTL_SLEEP, LCD_IOCTL_ON);
	}
	else if (state == LCD_OFF) {
		ioctl(display->file, LCD_IOCTL_SLEEP, LCD_IOCTL_OFF);
	}
}

void lcd_backlight(int state, struct display *display) {
	if (state == LCD_ON) {
		ioctl(display->file, LCD_IOCTL_BACKLIGHT, LCD_IOCTL_ON);
	}
	else if (state == LCD_OFF) {
		ioctl(display->file, LCD_IOCTL_BACKLIGHT, LCD_IOCTL_OFF);
	}
}

void lcd_onoff(int state,  struct display *display) {
	if (state == LCD_ON) {
		ioctl(display->file, LCD_IOCTL_ONOFF, LCD_IOCTL_ON);
	}
	else if (state == LCD_OFF) {
		ioctl(display->file, LCD_IOCTL_ONOFF, LCD_IOCTL_OFF);
	}
}
