/*
 * lcd_demo.h
 *
 *  Created on: May 30, 2010
 *      Author: celebro
 */

#ifndef LCD_DEMO_H_
#define LCD_DEMO_H_

void restore_terminal_settings(void);
void disable_waiting_for_enter(void);
char get_key();

void build_vars();

void clear_screen();
void intro_animation();

void switch_tab(int previous, int current);
int menu_selection(int position);

void demo_pixel();
void demo_line();
void demo_rect();

void demo_strings();
void demo_bitmap();
void demo_combined();

void demo_backlight();
void demo_sleep();
void demo_off();

void demo_pong();


#endif /* LCD_DEMO_H_ */
