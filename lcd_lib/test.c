/*
 * test.c
 *
 *  Created on: 7.5.2010
 *      Author: Jure
 */

#include <stdio.h>
#include "display_lib.h"
#include "lcd.h"

struct display* display;

int main(){
	printf("Get display\n");

	display = get_display();

	if (display == NULL) {
		printf("Failed to get display\n");
		return -1;
	}

	getc(stdin);

//	printf("Lcd rect\n");
//	lcd_set_rect(20, 20, 50, 50, FILL, WHITE, display);
//	getc(stdin);
//	lcd_set_rect(20, 60, 50, 90, FILL, RED, display);
//	getc(stdin);
//	lcd_set_rect(60, 20, 90, 50, FILL, BLUE, display);
//	getc(stdin);
//	lcd_set_rect(60, 60, 90, 90, FILL, ORANGE, display);

//	printf("Lcd pixel\n");
//	lcd_set_pixel(100, 100, WHITE, display);

//	lcd_set_line(2,0, 20, 78, RED, display);
//	getc(stdin);
//	lcd_set_line(30,19, 98, 34, BLUE, display);
//	getc(stdin);
//	lcd_set_line(130,129, 20, 60, GREEN, display);

	lcd_put_str("Test str 1\n", 10, 10, SMALL, YELLOW, display);
	getc(stdin);
	lcd_put_str("Test str 1\n", 30, 10, MEDIUM, GREEN, display);
	getc(stdin);
	lcd_put_str("Test str 1\n", 50, 10, LARGE, ORANGE, display);
	getc(stdin);

	printf("Release display\n");
	release_display(display);
}

