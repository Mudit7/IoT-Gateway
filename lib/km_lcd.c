#include<stdio.h>
#include "../inc/lcd.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

unsigned int *gpio2;

int Write_Gpio_Value(int Pin, int Val){

	//p = (unsigned long *)0X481AC000;
		if (Val == 1)
		*(unsigned long *)(gpio2+101) |= (1 << (Pin % 32));
		if (Val == 0)
		*(unsigned long *)(gpio2+100) = (1 << (Pin % 32));
	return 1;
}

static int KM_GPIO_Export(int pin)
{
#define BUFFER_MAX 3
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
 
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open export for writing!\n");
		return(-1);
	}
 
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}
 
static int KM_GPIO_Unexport(int pin)
{
	char buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int fd;
 
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return(-1);
	}
 
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

void LCD_Write_High_Nibble(unsigned char data )
{
	if(data & 0x10)
		Write_Gpio_Value(LCD_D4, 1); 
	else
		Write_Gpio_Value(LCD_D4, 0);
	
	if(data & 0x20)
        	Write_Gpio_Value(LCD_D5, 1);
	else 	
		Write_Gpio_Value(LCD_D5, 0);
	if(data & 0x40)
        	Write_Gpio_Value(LCD_D6, 1);
	else 
        	Write_Gpio_Value(LCD_D6, 0);
	if(data & 0x80)
        	Write_Gpio_Value(LCD_D7, 1);
	else 
         	Write_Gpio_Value(LCD_D7, 0);

	HD44780_EN(ON);
	Delay_HD44780(DELAY_EN);
	HD44780_EN(OFF);
}


void LCD_Write_Low_Nibble(unsigned char data ){
if(data & 0x01)
        { Write_Gpio_Value(LCD_D4, 1); }
else 
         { Write_Gpio_Value(LCD_D4, 0); }

if(data & 0x02)
        { Write_Gpio_Value(LCD_D5, 1); }
else
         { Write_Gpio_Value(LCD_D5, 0); }

if(data & 0x04)
        { Write_Gpio_Value(LCD_D6, 1); }
else
         { Write_Gpio_Value(LCD_D6, 0); }

if(data & 0x08)
        { Write_Gpio_Value(LCD_D7, 1); }
else
         { Write_Gpio_Value(LCD_D7, 0); }
HD44780_EN(ON);
Delay_HD44780(DELAY_EN);
HD44780_EN(OFF);
}

void HD44780_EN(char Mode){
	if(Mode == ON){
	Write_Gpio_Value(LCD_EN,1);
	return;
	}
	Write_Gpio_Value(LCD_EN,0);
}

void Delay_HD44780(unsigned int Delay){
	int i=0;
	for(i=0;i<Delay;i++)
	usleep(10000);
}


/* 4bit bus version */

void hd44780_write( unsigned char data ){
/* send the data bits - high nibble first */
	LCD_Write_High_Nibble( data );
	LCD_Write_Low_Nibble( data );
}

void hd44780_wr_cmd( unsigned char cmd ){
	Write_Gpio_Value(LCD_RS,0);
	hd44780_write( cmd );
}

void hd44780_wr_data( unsigned char data ){
	Write_Gpio_Value(LCD_RS,1);
	hd44780_write( data );
}

void HD44780_PutChar(unsigned char c){
hd44780_wr_data(c & 0xff);
}

void HD44780_GotoXY(unsigned char x, unsigned char y){
 	unsigned char copy_y=0;
	if(x > (HD44780_DISP_LENGTH-1))
	x = 0;
	if(y > (HD44780_DISP_ROWS-1))
		y = 0;
	switch(y) {
	case 0:  copy_y = 0x80; break;
	case 1:  copy_y = 0xc0; break;
	case 2:  copy_y = 0x94; break;
	case 3:  copy_y = 0xd4; break;
	 }
hd44780_wr_cmd(x + copy_y);
}

void HD44780_PutStr(char *str){
 unsigned int i=0;
do{
HD44780_PutChar(str[i]);
i++;
}while(str[i]!='\0');
}

void KM_LCD_ClrScr(void){
	hd44780_wr_cmd(HD44780_CMD_CLEAR);
}

void KM_LCD_Str_XY(char X, char Y, char *Ptr){
	HD44780_GotoXY(X,Y);
	HD44780_PutStr(Ptr);
}
 
int KM_LCD_Init(void)
{
	char ch = 'n';
        int i,fd;
        unsigned int addr = 0x481AC000UL;
	const size_t MAP_SIZE = 4096UL;
        const size_t MAP_MASK = MAP_SIZE - 1;

	KM_GPIO_Export(66); // Enable GPIO 2 controller otherwise memory fault error occurs.

	fd = open("/dev/mem", O_RDWR | O_SYNC);
        if (fd == -1) {
                perror("can't open /dev/mem");
		return -1;
                abort();
        }
	printf("Before mmap\n");
        gpio2 = mmap(0, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED,fd, addr & ~MAP_MASK);
        if (gpio2 == (void*)-1) {
                perror("mmap() failed");
		return -1;
                abort();
        }
	printf("After mmapn gpio2:%x\n",gpio2);
	//gpio2=gpio2+77;
	//printf("After mmap gpio2 ** :%x\n",gpio2);
        //unsigned char ret_val = *(gpio2 + (addr & MAP_MASK));
	
        //unsigned int ret_val = *(gpio2);
	//printf("*gpio2:%x\n",*(unsigned int *)gpio2);

	/* Alternately turn the LEDs on and off */
	//gpio2 = (unsigned long *)0X481AC000;
	*(unsigned long *)(gpio2+77) &= ~(1 << (66 % 32));  //set output direction
	*(unsigned long *)(gpio2+77) &= ~(1 << (67 % 32));  //set output direction
	*(unsigned long *)(gpio2+77) &= ~(1 << (68 % 32));  //set output direction
	*(unsigned long *)(gpio2+77) &= ~(1 << (69 % 32));  //set output direction
	*(unsigned long *)(gpio2+77) &= ~(1 << (86 % 32));  //set output direction
	*(unsigned long *)(gpio2+77) &= ~(1 << (87 % 32));  //set output direction
	*(unsigned long *)(gpio2+77) &= ~(1 << (88 % 32));  //set output direction
	*(unsigned long *)(gpio2+77) &= ~(1 << (89 % 32));  //set output direction
	
	Write_Gpio_Value(BUZZ,1);	//Buzzer ON
	Write_Gpio_Value(LCD_EN,0);	//Clear EN
	Write_Gpio_Value(LCD_RS,0);	//Clear RS
	Write_Gpio_Value(LCD_RW,0);	//Clear RW
	usleep(10);			//Tas=3uSec
	Write_Gpio_Value(LCD_EN,1);	//Set	EN
	usleep(10);			//Tas=1uSec
	Write_Gpio_Value(LCD_EN,0);	//Clear	EN
	usleep(10);			//Tas=1uSec

	for(i=0;i<3;i++){
	LCD_Write_High_Nibble(HD44780_CMD_RESET);
	usleep(10);
	}

	LCD_Write_High_Nibble(HD44780_CMD_FUNCTION );
	hd44780_function( HD44780_CONF_BUS, HD44780_CONF_LINES, HD44780_CONF_FONT );
	hd44780_display( HD44780_DISP_ON, HD44780_DISP_CURS_OFF, HD44780_DISP_BLINK_OFF );
	hd44780_clear();
	hd44780_entry( HD44780_ENTRY_ADDR_INC, HD44780_ENTRY_SHIFT_CURS );
	KM_LCD_ClrScr();
	usleep(10);			//Tas=1uSec
	KM_LCD_Str_XY(3,0,"Welcome to");
	KM_LCD_Str_XY(1,1,"Kernel Masters");
	
	Write_Gpio_Value(BUZZ,0);	//Buzzer OFF
        close(fd);
	return 0;
}

void KM_LCD_Exit(void)
{
	const size_t MAP_SIZE = 4096UL;
	KM_GPIO_Unexport(66);
        if (munmap(gpio2, MAP_SIZE) != 0)
        {
                perror("munmap() failed");
                abort();
        }
}


