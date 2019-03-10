void KM_LCD_Init(void);
void KM_LCD_Write_Cmd( unsigned char cmd );
void KM_LCD_Write_Char( unsigned char data );
void KM_LCD_Write_Str(char *str);

#define GPIO_PORTD_DATA_R (*((volatile unsigned long *)0x400073FC)) 
#define GPIO_PORTD_DIR_R (*((volatile unsigned long *)0x40007400))
#define GPIO_PORTD_AFSEL_R (*((volatile unsigned long *)0x40007420))
#define GPIO_PORTD_DEN_R (*((volatile unsigned long *)0x4000751C))
#define GPIO_PORTD_AMSEL_R (*((volatile unsigned long *)0x40007528))
#define GPIO_PORTD_DR8R_R (*((volatile unsigned long *)0x40007508))
#define GPIO_PORTD_PCTL_R (*((volatile unsigned long *)0x4000752C))
#define GPIO_PORTD_PU_R (*((volatile unsigned long *)0x40007510))
#define GPIO_PORTD_LOCK_R (*((volatile unsigned long *)0x40007520))
#define GPIO_PORTD_CR_R (*((volatile unsigned long *)0x40007524))

/********************************************************************/
#define GPIO_PORTA_DATA_R (*((volatile unsigned long *)0x400043FC)) 
#define GPIO_PORTA_DIR_R (*((volatile unsigned long *)0x40004400))
#define GPIO_PORTA_AFSEL_R (*((volatile unsigned long *)0x40004420))
#define GPIO_PORTA_DEN_R (*((volatile unsigned long *)0x4000451C))
#define GPIO_PORTA_AMSEL_R (*((volatile unsigned long *)0x40004528))
#define GPIO_PORTA_PCTL_R (*((volatile unsigned long *)0x4000452C))
# define GPIO_PORTA_PU_R (*((volatile unsigned long *)0x40004510))
#define SYSCTL_RCGC2_R (*((volatile unsigned long *)0x400FE108))
#define GPIO_PORTA_LOCK_R (*((volatile unsigned long *)0x40004520))
#define GPIO_PORTA_CR_R (*((volatile unsigned long *)0x40004524))
	/****************************************************/
#define SYSCTL_RCGCGPIO_R  (*((volatile unsigned long *)0x400FE608))
#define SYSCTL_RCC2_R (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RIS_R (*((volatile unsigned long *)0x400FE050))
#define SYSCTL_RCC_R (*((volatile unsigned long *)0x400FE060))
#define SYSCTL_RCGC2_GPIOA      0x00000001  // port C Clock Gating Control
#define SYSCTL_RCGC2_GPIOD      0x00000008  // port C Clock Gating Control

/**************************************************
LCD_D4---->PD0
LCD_D5---->PD1
LCD_D6---->PD2
LCD_D7---->PD3

LCD_RS--->PA7
LCD_EN--->PA6
LCD_RW--->PA5
**************************************************/
#define SET 1
#define CLR 0
#define VOLTAGE_D0(X)	( (X) ? (GPIO_PORTD_DATA_R |= (1<<0)) :  (GPIO_PORTD_DATA_R &= ~(1<<0)) )
#define VOLTAGE_D1(X)	( (X) ? (GPIO_PORTD_DATA_R |= (1<<1)) :  (GPIO_PORTD_DATA_R &= ~(1<<1)) )
#define VOLTAGE_D2(X)	( (X) ? (GPIO_PORTD_DATA_R |= (1<<2)) :  (GPIO_PORTD_DATA_R &= ~(1<<2)) )
#define VOLTAGE_D3(X)	( (X) ? (GPIO_PORTD_DATA_R |= (1<<3)) :  (GPIO_PORTD_DATA_R &= ~(1<<3)) )
/******************************************************/
#define RS(X)	       ( (X) ? (GPIO_PORTA_DATA_R |= (1<<7)) :  (GPIO_PORTA_DATA_R &= ~(1<<7)) )
#define RW(X) 				( (X) ? (GPIO_PORTA_DATA_R |= (1<<5)) :  (GPIO_PORTA_DATA_R &= ~(1<<5)) )
#define EN(X)					( (X) ? (GPIO_PORTA_DATA_R |= (1<<6)) :  (GPIO_PORTA_DATA_R &= ~(1<<6)) )
/*************************************************************************/
void sec_Delay(unsigned long del);
void Init_PLL(void);
void Init_LCD(void);
void Delay(unsigned long del);
void write_high_nibble( unsigned char data );
void write_low_nibble( unsigned char data );
void write( unsigned char data );
void write_cmd( unsigned char cmd );
void write_data( unsigned char data );
void Putchar(unsigned char c);
void write_str(char *str);
unsigned int j,i;
/************************************************************/

void KM_LCD_Init(void)
	{	
		//SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD;

	SYSCTL_RCGCGPIO_R |= 0X00000008;
    Delay(4);
	GPIO_PORTD_LOCK_R = 0x4C4F434B;				
	GPIO_PORTD_CR_R = 0x0F;							
	GPIO_PORTD_DIR_R = 0x0F;						
	GPIO_PORTD_DEN_R = 0x0F;			
	//	SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;
		
SYSCTL_RCGCGPIO_R |= 0X00000001;
Delay(4);
	GPIO_PORTA_LOCK_R = 0x4C4F434B;
	GPIO_PORTA_CR_R |= 0xE0;					
	GPIO_PORTA_DIR_R |= 0xE0;				
	GPIO_PORTA_DEN_R |= 0xE0;				
/**************************************************************************/	
	/* clear control bits */
	EN(CLR);
	RS(CLR);
	RW(CLR);
/***************************************************/	
  sec_Delay(16);//Delay(1279000);
  write_high_nibble(0X30);
	sec_Delay(5);//Delay(514000);
 write_high_nibble(0X30);
  sec_Delay(1);//Delay(102800);
 write_high_nibble(0X30);
  write_high_nibble(0X20);
	//write_cmd(0x28);
	KM_LCD_Write_Cmd(0x0C); // Display ON, Cursor off
  KM_LCD_Write_Cmd(0X01); // Display Clear
	//write_cmd(0X06);	
	//write_cmd(0X01);	
    }
//////////////////////////////////////////////////////////////////////////////////////////////////
void Delay(unsigned long del)
{
	unsigned long i=0;
	while(i<del)
		i++;
}

void sec_Delay(unsigned long del)
{
	for(i=0; i<del;i++)
	Delay(102800); // 1ms
}
////////////////////////////////////////////////////////////////////////////
void write_high_nibble( unsigned char data )
{
if ( data & 0x10 ) {
		VOLTAGE_D0(SET);	
	} else {
		VOLTAGE_D0(CLR);
	}
	if ( data & 0x20 ) {
		VOLTAGE_D1(SET);	
	} else {
		VOLTAGE_D1(CLR);
	}
	if ( data & 0x40 ) {
		VOLTAGE_D2(SET);	
	} else {
		VOLTAGE_D2(CLR);
	}
	if ( data & 0x80 ) {
		VOLTAGE_D3(SET);
	} else {
		VOLTAGE_D3(CLR);
	}
    /* set the EN signal */
    EN(SET);
   Delay(200000);
    /* reset the EN s */
    EN(CLR);
}
/**************************************************/
void write_low_nibble( unsigned char data )
{
	if ( data & 0x01 ) {
		VOLTAGE_D0(SET);	
	} else {
		VOLTAGE_D0(CLR);
	}
	if ( data & 0x02 ) {
		VOLTAGE_D1(SET);	
	} else {
		VOLTAGE_D1(CLR);
	}
	if ( data & 0x04 ) {
		VOLTAGE_D2(SET);	
	} else {
		VOLTAGE_D2(CLR);
	}
	if ( data & 0x08 ) {
		VOLTAGE_D3(SET);	
	} else {
		VOLTAGE_D3(CLR);
	}

    /* set the EN  */ 
    EN(SET);
	//sec_Delay(1);
		Delay(200000);
    /* reset the EN */
    EN(CLR);
}
/**************************************************/
 void write( unsigned char data )
{
               
	write_high_nibble( data );
	write_low_nibble( data );
}
//////////////////////////////////////////////////////////
void KM_LCD_Write_Cmd( unsigned char cmd )
{
	RS(CLR);
	write( cmd );
}
///////////////////////////////////////////////
void KM_LCD_Write_Char( unsigned char data )
{
	RS(SET);
	write( data );
}
/**************************************************/
void KM_LCD_Write_Str(char *str)
{
 unsigned int i=0;
	do
	{
		KM_LCD_Write_Char(str[i]);
		i++;
	}while(str[i]!='\0');
}

void LCD_Print_Number(int num){
	int i;
	char val[5] = {0};
	i = num;
	val[0] = (i/1000) + 48;
	i = i % 1000;
	val[1] = (i/100) + 48;
	i = i % 100;
	val[2] = (i/10) + 48;
	i = i % 10;
	val[3] = i + 48;
	val[4] = 0;
	 KM_LCD_Write_Str(val);
}
