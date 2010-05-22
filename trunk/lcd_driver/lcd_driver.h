#ifndef LCD_DRIVER_H_
#define LCD_DRIVER_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
//#include <linux/errno.h>

/* For get_user and put_user */
#include <asm/uaccess.h>

/* Char device registration and udev creation */
#include <linux/fs.h>
#include <linux/cdev.h>


#include <asm/io.h>
#include <mach/hardware.h>
//#include <mach/gpio.h>

/* Prefix in /var/log/messages*/
#define PREFIX "lcd driver: "
#define IOCTL_PREFIX "ioctl: "

#define BUF_SIZE	2048

#define LCD_MISO	BIT0
#define LCD_MOSI	BIT1
#define LCD_CLOCK	BIT2
#define LCD_CS		BIT3

#define LCD_PWR		BIT6
#define LCD_BL		BIT7
#define LCD_RESET	BIT8

/* Internal character device presentation */
struct char_device {
	char *name;
	int major;
	struct file_operations *fops;
	struct cdev cdev;
};

/* Module functions */
static int __init lcd_driver_init (void);
static void __exit lcd_driver_exit (void);

/* Char device creation an release */
static struct char_device* char_device_create(char *name, struct file_operations *fops);
static void char_device_release(struct char_device *cdevice);

/* Char device operations */
static int		lcd_open(struct inode *, struct file *);
static int		lcd_release(struct inode *, struct file *);
static ssize_t	lcd_read(struct file *, char *, size_t, loff_t *);
static ssize_t	lcd_write(struct file *, const char  __user *, size_t, loff_t *);
static int		lcd_ioctl(struct inode *, struct file *, unsigned int, unsigned long);

/* Other functions */
static void lcd_delay(int n);
static void init_controllers(void);
static void release_controllers(void);
static void init_lcd(void);
static void sleep_lcd(void);
static void wake_lcd(void);
static void write_spi_command(volatile unsigned int command);
static void write_spi_data(volatile unsigned int data);

static void lcd_set_pixel(int x, int y, int color);
static void lcd_set_rect(int x0, int y0, int x1, int y1, int fill, int color);
static void lcd_set_line (int x0, int y0, int x1, int y1, int color);

#endif /* LCD_DRIVER_H_ */
