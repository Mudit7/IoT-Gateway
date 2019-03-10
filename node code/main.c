// main.c for node
// Runs on LM4F120/TM4C123

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "PLL.h"
#include "uart.h"
#include "BitField.h"
#include<string.h>

#define NVIC_EN0_R              (*((volatile unsigned long *)0xE000E100))
#define NVIC_PRI0_R             (*((volatile unsigned long *)0xE000E400))

#define DEV_ID '1'					//Device id of the node

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void SysTick_Init(void);
void _delay_ms(unsigned long );
void _delay_us(unsigned long );
void KM_LCD_Init(void);
void KM_LCD_Write_Cmd( unsigned char cmd );
void KM_LCD_Write_Char( unsigned char data );
void KM_LCD_Write_Str(char *str);
void LCD_Print_Number(int );
char temp1[300], rx_flag=0, tx_flag=0;
extern uint32_t ADCvalue;
//char temp1[300], rx_flag=0, tx_flag=0;
uint8_t bits[5];
char val[4] = {0};
int temp;
int config_flag=0;
char str[20]="RS485";
//debug code
// This program periodically samples ADC0 channel 0 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.

void Port_Init(void)
{
//	SYSCTL->RCGCGPIO	 =	( Bit1 |  Bit4 | Bit5);
	SYSCTL->RCGC2	 =	( Bit0 | Bit1 | Bit2 | Bit3 | Bit4 | Bit5 );
	_delay_ms(100);
	
		//PortA Initialisation - LCD Control Lines - PA5,PA6 & PA7
	GPIOA->CR				|=		 	 (Bit5 | Bit6 | Bit7 | Bit3);
	GPIOA->DIR				|=		 	 (Bit5 | Bit6 | Bit7);
	GPIOA->AFSEL			|=			 (Bit0 | Bit1);
	GPIOA->AFSEL			&=			 ~(Bit5 | Bit6 |Bit7 | Bit3);
//	GPIOA->PCTL				 =			 ((GPIOA->PCTL & 0xFFFFFF00) | 0x00000011);
	GPIOA->PCTL				=			  0;
	GPIOA->AMSEL			&=			~(Bit0 | Bit1 | Bit5 | Bit6 | Bit7 | Bit3);
	GPIOA->DEN				|=			 (Bit0 | Bit1 | Bit5 | Bit6 | Bit7 | Bit3);
	
	
	//PortB - PB0,PB1 Initialisation - MUX A & MUX B
	GPIOB->CR					|=				(Bit4);
	GPIOB->DIR				|=		 	 (Bit0 | Bit1);
	GPIOB->AFSEL			&=		  ~(Bit0 | Bit1);
	GPIOB->PCTL		  	&=	    ~(0x000F00FF);
	GPIOB->AMSEL			&=			~(Bit0 | Bit1);
	GPIOB->DEN				|=			 (Bit0 | Bit1 | Bit4);
	
	 // PE0 - RS485 Control Line
  		GPIOE->DIR			|=		 0x01; 
		  GPIOE->AFSEL    &=		~0x01;
			GPIOE->DEN			|=		 0x01;
			GPIOE->AMSEL		&=		~0x01;
			GPIOE->PCTL			&=		~0x00000F0F;
	
	GPIOF->LOCK			 =		lock;
	GPIOF->CR				 =		(Bit0 | Bit1 | Bit2 | Bit3 | Bit4);
	GPIOF->DEN			|=		(Bit0 | Bit1 | Bit2 | Bit3 | Bit4);
	GPIOF->DIR			|=		(Bit1 | Bit2 | Bit3);
	GPIOF->DIR			&=		~(Bit0 | Bit4);
	GPIOF->AMSEL		&=		~(Bit0 | Bit1 | Bit2 | Bit3 | Bit4);
	GPIOF->AFSEL    &=		0;
	GPIOF->PCTL			&=		0;
	GPIOF->PUR			|=		(Bit0 | Bit4);
}
char* itoa(uint8_t a,char func)
{
	unsigned int i=0,j=0;
	int num=a;
	char numstr[10];
	char str[10];
	static char newstr[16];
		newstr[0]='<';
		newstr[1]=DEV_ID;
		newstr[2]=func;
		newstr[5]='>';
		newstr[6]=' ';
		newstr[7]=' ';
   	newstr[8]=' ';
	newstr[9]=' ';
	newstr[10]=' ';
	newstr[11]=' ';
	newstr[12]=' ';
	newstr[13]=' ';
	newstr[14]=' ';
	newstr[15]=0;
	if(num==0)					//Exception
	{
		newstr[3]='0';
		newstr[4]='0';
		//_delay_ms(100);
		
		return newstr;
	} 
   while(num!=0)
	 {
		 numstr[i]=num%10+48;
		 i++;
		 num=num/10;
	 }
	 
	 numstr[i]=0;
  while(numstr[j]!=0)
	{
		str[j]=numstr[i-j-1];
		j++;
	}		
  	newstr[3]=str[0];
		newstr[4]=str[1];
 return newstr;
}
void HIGH()
{
	GPIOB->DATA|=(Bit4);
}
void LOW()
{
	GPIOB->DATA&=~(Bit4);
}

int checkLow()
{
	if((GPIOB->DATA)&0x10)
	return 0;
	else 
		return 1;
}
int checkHigh()
{
	if((GPIOB->DATA)&0x10)
	return 1;
	else 
		return 0;
}

void Send_Start_Signal()
{
	unsigned int loopCnt=300;
	int i;
	for(i=0;i<5;i++)									//Initializing buffer to 0
		bits[i]=0;
		
		 GPIOB->DIR |= 0x10;						//Making PB4 as output
		LOW();														//Controller sends a start signal to the DHT11 module
		_delay_ms(20);
		HIGH();														//Pulling the data line HIGH such that the DHT11 module starts behaving as input to controller
  	_delay_us(40);
		GPIOB->DIR &= ~0x10;				//Making PB4 as input in order to receive the temperature and humidity values		
		loopCnt=300;
		
		
		while(checkLow())									//Checking whether DHT sends low level response signal which lasts for about 80usec
		{
			if(loopCnt--==0) 
			{
		//		KM_LCD_Write_Cmd(0x01);
		//    KM_LCD_Write_Str("Error");    //If the pulse exceed 100us then error
				break;
			}
		}
		loopCnt=300;											//loopCnt 300 ~= 100us
    while(checkHigh())								//Checking whether DHT11 pulls up the voltage line for 80us and prepares for data transmission
    {		
		  if(loopCnt--==0) 
			{
		//		KM_LCD_Write_Cmd(0x01);
		 //   KM_LCD_Write_Str("Error"); 		//If the pulse exceed 100us then error
				break;
			}
		}
}

void Get_Data()
{
unsigned int loopCnt=300;
uint8_t cnt=7;
uint8_t idx=0;
int i=0;
												/****************DHT11 TRANSMITS DATA***************************/
for(i=0;i<40;i++)
{
	int count=0;	
	loopCnt=300;
	while(checkLow())									//Getting past the initial 50us low pulse
	{
			if(loopCnt--==0) 
			{
			//	KM_LCD_Write_Cmd(0x01);
		  //  KM_LCD_Write_Str("Error");		//If the pulse exceed 100us then error

				break;
			}
	} 
		
	count=0;
	loopCnt=300;
	while(checkHigh())						//Measuring the duration of high pulse
	{
			count++;
			if(loopCnt--==0) 
			{
			//	KM_LCD_Write_Cmd(0x01);
		   // KM_LCD_Write_Str("Error");   //If the pulse exceed 100us then error
				break;
			}
	}
	if(count>100)             	//Checking whether the data bit is 0 or 1 ==> 100 count ~= 45us
	{
			bits[idx]|=(1<<cnt);
	}
				
	if(cnt==0)
	{
			cnt=7;
			idx++;
	}
	else
			cnt--;
}
}

int main(void){
	int i;
  PLL_Init();               // 80 MHz system clock
	SysTick_Init();
  Port_Init();
	UART_Init();             // initialize UART
	KM_LCD_Init();
	KM_LCD_Write_Cmd(0x01);
	GPIOB->DATA |= Bit1;	
	GPIOB->DATA |= Bit0;
  rx_flag=0;
	 EnableInterrupts();
	 KM_LCD_Write_Cmd(0x01);
	 KM_LCD_Write_Str("Node  -> Zigbee");
	 KM_LCD_Write_Cmd(0x84);
	 KM_LCD_Write_Char(DEV_ID);
	 strcpy(str,"Zigbee");
	while(1){
		
	     	 for (i=0; i<= 10; i++) { temp1[i]=0x00; }   // clear Buffer
         while (rx_flag == 0);
  			 rx_flag=0;
				 Send_Start_Signal();
							Get_Data();
//				  KM_LCD_Write_Cmd(0x01);
	 //       KM_LCD_Write_Str("Node  -> ");
//				  KM_LCD_Write_Str(str);
	//				KM_LCD_Write_Cmd(0x84);
		//			KM_LCD_Write_Char(DEV_ID);
	//	     if (temp1[0] == '<')
		//		 {
					 if( temp1[1] == DEV_ID || temp1[1]=='0')
					 {
					   if (temp1[2] == 'T') {
							 KM_LCD_Write_Cmd(0xC0);
							 KM_LCD_Write_Str("Temp: ");
						  GPIOE->DATA |=  Bit0;
							LCD_Print_Number(bits[2]); 
							 KM_LCD_Write_Str("        ");
							  UART_OutString(itoa(bits[2],'T'));
						 	 _delay_ms(90);
							GPIOE->DATA &=  ~(Bit0);		
							}	
						 else if (temp1[2] == 'H') {
							 KM_LCD_Write_Cmd(0xC0);
							 KM_LCD_Write_Str("Hum: ");
						  GPIOE->DATA |=  Bit0;
							 	LCD_Print_Number(bits[0]); 
							 KM_LCD_Write_Str("        ");
							 UART_OutString(itoa(bits[0],'H'));
						 	 _delay_ms(90);
							GPIOE->DATA &=  ~(Bit0);		
							}
						 else if (temp1[2] == 'R') {
							 KM_LCD_Write_Cmd(0xC0);
							 KM_LCD_Write_Str("Conf. for RS485");
							 _delay_ms(50);
							 GPIOB->DATA =0x02;
							 _delay_ms(50);
							 strcpy(str,"RS485");	
							}
						 else if (temp1[2] == 'Z') {
							 KM_LCD_Write_Cmd(0xC0);
							 KM_LCD_Write_Str("Conf. for Zig.");
							  _delay_ms(50);
						   GPIOB->DATA = 0x03;
							 _delay_ms(50);
							 strcpy(str,"ZigBee");
							}
						  else if (temp1[2] == 'W')
							{
								
							}
								
						  else
							{
								KM_LCD_Write_Cmd(0xC0);
								KM_LCD_Write_Str("Wrong Code");
					//			KM_LCD_Write_Str(temp1);
							}	
				 } 
				 else
				 {
					  KM_LCD_Write_Cmd(0xC0);
					 KM_LCD_Write_Str("Wrong Dev ID");
					}
							
		/*	 }
				 else
				 {
					 KM_LCD_Write_Cmd(0xC0);
			   	 KM_LCD_Write_Str("Invalid Data");
				 }	*/			 
			}
}	
