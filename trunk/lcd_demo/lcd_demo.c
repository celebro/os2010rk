#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>

#include "display_lib.h"

struct display* display;

/* Key processing */
#define ENTER	0
#define UP		1
#define DOWN	2
#define LEFT	3
#define RIGHT	4
#define QUIT	5

static struct termios oldt;

void restore_terminal_settings(void) {
	tcsetattr(0, TCSANOW, &oldt); /* Apply saved settings */
}

void disable_waiting_for_enter(void) {
	struct termios newt;

	/* Make terminal read 1 char at a time */
	tcgetattr(0, &oldt); /* Save terminal settings */
	newt = oldt; /* Init new settings */
	newt.c_lflag &= ~(ICANON | ECHO); /* Change settings */
	tcsetattr(0, TCSANOW, &newt); /* Apply settings */
	atexit(restore_terminal_settings); /* Make sure settings will be restored when program ends  */
}

char get_key() {
	int ch;

	while (1) {
		ch = getchar();
		if (ch == 27) {
			ch = getchar();
			if (ch == 91) {
				ch = getchar();
				if (ch == 65)
					return UP;
				if (ch == 66)
					return DOWN;
				if (ch == 67)
					return RIGHT;
				if (ch == 68)
					return LEFT;
			}
		} else if (ch == 10)
			return ENTER;
		else if (ch == 'Q' || ch == 'q') {
			return QUIT;
			//program_running = 0;
		}
	}

	return 0;
}

/* Intro animation */
void intro_animation() {
	int i;
	int menu_size = 18;
	int spacing = 15;
	int str_vspace = 2;
	int str_hspace = 4;
	int color = ORANGE;
	int str_color = RED;
	int str_size = LARGE;
	int step = 4;

	int m0 = 7;
	int m1 = m0 + menu_size + spacing;
	int m2 = m1 + menu_size + spacing;
	int m3 = m2 + menu_size + spacing;

	int m0s = 100;
	int m1s = 85;
	int m2s = 70;
	int m3s = 55;

	/* TODO a nice bmp */
	for (i = -m0s; i <= 0; i = i+step) {
		lcd_set_rect(m0, i, m0 + menu_size, i+m0s, FILL, color, display);
		lcd_put_str("Menu 0", m0+str_vspace, i+str_hspace, str_size, str_color, display);
	}

	for (i = -m1s; i <= 0; i = i+step) {
		lcd_set_rect(m1, i, m1 + menu_size, i+m1s, FILL, color, display);
		lcd_put_str("Menu 1", m1+str_vspace, i+str_hspace, str_size, str_color, display);
	}

	for (i = -m2s; i <= 0; i = i+step) {
		lcd_set_rect(m2, i, m2 + menu_size, i+m2s, FILL, color, display);
		lcd_put_str("Menu 2", m2+str_vspace, i+str_hspace, str_size, str_color, display);
	}

	for (i = -m3s; i <= 0; i = i+step) {
		lcd_set_rect(m3, i, m3 + menu_size, i+m3s, FILL, color, display);
		lcd_put_str("Menu 3", m3+str_vspace, i+str_hspace, str_size, str_color, display);
	}
}

void switch_tab(int previous, int current){
	int menu_size = 18;
	int spacing = 15;
	int str_vspace = 2;
	int str_hspace = 4;
	int color = ORANGE;
	int str_color = RED;
	int str_size = LARGE;

	int mx = 7 + (3 - previous) * 33;		//draw previously selected tab
	int mxs = 100 - (previous * 15);

	lcd_set_rect(mx, 0, mx + menu_size, mxs, FILL, color, display);
	lcd_put_str("Menu X", mx+str_vspace, str_hspace, str_size, str_color, display);

	mx = 7 + (3 - current) * 33;			//draw currently selected tab
	mxs = 100 - (current * 15);

	color = RED;
	str_color = ORANGE;

	lcd_set_rect(mx, 0, mx + menu_size, mxs, FILL, color, display);
	lcd_put_str("Menu X", mx+str_vspace, str_hspace, str_size, str_color, display);

}

int menu_selection(char key, int position){
	int menu_size = 3;

	if((key == DOWN) && position==menu_size)	//end is reached, return current position
		return position;
	if((key == UP)&& position == 0)
		return position;

	switch (key){
		case ENTER:
			/* TODO select current menu*/
		case DOWN:
			switch_tab(position,position+1);
			position++;
		case UP:
			switch_tab(position,position-1);
			position--;
		default:
			break;
	}

	return position;
}

int main(){

	display = get_display();
	if (display == NULL) {
		printf("Failed to get display\n");
		return -1;
	}

	disable_waiting_for_enter();

	intro_animation();

	char key = 0;
	int position = 0;

	while((key = get_char)!= QUIT){
		position = menu_selection(key,position);
	}


	restore_terminal_settings();
	release_display(display);
}
