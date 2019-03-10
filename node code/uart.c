// UART.c
// Simple device driver for the UART works as polling method.  

#include "uart.h"
#include "TM4C123GH6PM.h"

void EnableInterrupts(void);  // Enable interrupts
void SysTick_Init(void);
void _delay_ms(unsigned long );

extern unsigned char temp1[50],rx_flag,tx_flag;

#define GPIO_PORTC_AFSEL_R      (*((volatile unsigned long *)0x40006420))
#define GPIO_PORTC_DEN_R        (*((volatile unsigned long *)0x4000651C))
#define GPIO_PORTC_AMSEL_R      (*((volatile unsigned long *)0x40006528))
#define GPIO_PORTC_PCTL_R       (*((volatile unsigned long *)0x4000652C))
#define UART1_DR_R              (*((volatile unsigned long *)0x4000D000))
#define UART1_FR_R              (*((volatile unsigned long *)0x4000D018))
#define UART1_IBRD_R            (*((volatile unsigned long *)0x4000D024))
#define UART1_FBRD_R            (*((volatile unsigned long *)0x4000D028))
#define UART1_LCRH_R            (*((volatile unsigned long *)0x4000D02C))
#define UART1_CTL_R             (*((volatile unsigned long *)0x4000D030))
#define UART1_IM_R              (*((volatile unsigned long *)0x4000D038))
#define UART1_RIS_R             (*((volatile unsigned long *)0x4000D03C))
#define UART1_ICR_R             (*((volatile unsigned long *)0x4000D044))
#define UART1_IFLS_R            (*((volatile unsigned long *)0x4000D034))
	
#define NVIC_EN0_R              (*((volatile unsigned long *)0xE000E100))  // IRQ 0 to 31 Set Enable Register
#define NVIC_PRI1_R             (*((volatile unsigned long *)0xE000E404))  // IRQ 28 to 31 Priority Register

#define UART_FR_TXFF            0x00000020  // UART Transmit FIFO Full
#define UART_FR_RXFE            0x00000010  // UART Receive FIFO Empty
#define UART_FR_RXFF            0x00000040  // UART Receive FIFO Empty
#define UART_LCRH_WLEN_8        0x00000060  // 8 bit word length
#define UART_LCRH_FEN           0x00000010  // UART Enable FIFOs
#define UART_CTL_UARTEN         0x00000001  // UART Enable
#define UART_IFLS_RX1_8         0x00000020  // RX FIFO >= 1/8 full
#define UART_IFLS_TX1_8         0x00000004  // TX FIFO <= 1/8 full
#define UART_IM_RTIM            0x00000040  // UART Receive Time-Out Interrupt
                                            // Mask
#define UART_IM_TXIM            0x00000020  // UART Transmit Interrupt Mask
#define UART_IM_RXIM            0x00000010  // UART Receive Interrupt Mask
#define UART_RIS_RTRIS          0x00000040  // UART Receive Time-Out Raw
                                            // Interrupt Status
#define UART_RIS_TXRIS          0x00000020  // UART Transmit Raw Interrupt
                                            // Status
#define UART_RIS_RXRIS          0x00000010  // UART Receive Raw Interrupt
                                            // Status
#define UART_ICR_RTIC           0x00000040  // Receive Time-Out Interrupt Clear
#define UART_ICR_TXIC           0x00000020  // Transmit Interrupt Clear
#define UART_ICR_RXIC           0x00000010  // Receive Interrupt Clear
#define SYSCTL_RCGC1_R          (*((volatile unsigned long *)0x400FE104))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))
#define SYSCTL_RCGC1_UART1      0x00000002  // UART1 Clock Gating Control
#define SYSCTL_RCGC2_GPIOC      0x00000004  // port C Clock Gating Control

#define NVIC_EN0_INT6           0x00000040  // Interrupt 6 enable

//------------UART_Init------------
// Initialize the UART for 115,200 baud rate (assuming 80 MHz UART clock),
// 8 bit word length, no parity bits, one stop bit, FIFOs enabled
// Input: none
// Output: none

void UART_Init(void){
	//SYSCTL->RCGCUART |=  0x00000002 ; //UART1 Clock Enable
	//SYSCTL->RCGCGPIO |=  0x00000004 ; //Port C Clock Enable
  SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART1; // activate UART1
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC; // activate port C
	_delay_ms(100);
		GPIO_PORTC_AFSEL_R |= 0x30;           // enable alt funct on PC5-4
  GPIO_PORTC_DEN_R |= 0x30;             // enable digital I/O on PC5-4
                                        // configure PC5-4 as UART1
  GPIO_PORTC_PCTL_R = (GPIO_PORTC_PCTL_R&0xFF00FFFF)+0x00220000;
  GPIO_PORTC_AMSEL_R &= ~0x30;          // disable analog functionality on PC5-4
	
  UART1_CTL_R &= ~UART_CTL_UARTEN;      // disable UART
  UART1_IBRD_R = 43;//130(38400); // 80,000,000/(16*38400)) = 130.20833  // 80,000,000/(16*115,200)) = 43.40278
  UART1_FBRD_R = 26;//13(38400);  //6-bbit fraction, round(0.20833 * 64) = 26
                                        // 8 bit word length (no parity bits, one stop bit, FIFOs)
  UART1_LCRH_R = (UART_LCRH_WLEN_8);// | UART_LCRH_FEN);
	// The below line enables to software Loopback
	//UART1_CTL_R |= 0x00000080;       // Loop Back Enable
	UART1_IFLS_R &= ~0x3F;                // clear TX and RX interrupt FIFO level fields
                                        // configure interrupt for TX FIFO <= 1/8 full
                                        // configure interrupt for RX FIFO >= 1/8 full
 // UART1_IFLS_R += (UART_IFLS_TX1_8|UART_IFLS_RX1_8);
	
  UART1_IM_R |= (UART_IM_RXIM|UART_IM_TXIM|UART_IM_RTIM);
	UART1_CTL_R |= UART_CTL_UARTEN;       // enable UART


	NVIC_PRI1_R = (NVIC_PRI1_R&0xFF00FFFF)|0x00400000; // bits 23 - 21
 NVIC_EN0_R = NVIC_EN0_INT6;           // enable interrupt 6 in NVIC
 //EnableInterrupts();
}

void UART1_Handler(void){
	int i;
  if(UART1_RIS_R&UART_RIS_TXRIS){       // hardware TX FIFO <= 2 items
		tx_flag=1;
	  UART1_ICR_R = UART_ICR_TXIC;        // acknowledge TX FIFO
  }
  if(UART1_RIS_R&UART_RIS_RXRIS){       // hardware RX FIFO >= 2 items
		temp1[0] = UART1_DR_R;
		if (temp1[0] == 0x3C)
		{	
		  for (i=1; i<= 10 ; i++) {
		  temp1[i] = UART_InChar();
			if (temp1[i] == '>') {
				rx_flag=1;
				break;
			}
		}
	  } 
		UART1_ICR_R = UART_ICR_RXIC;        // acknowledge RX FIFO
   }
  if(UART1_RIS_R&UART_RIS_RTRIS){       // receiver timed out
		for (i=0; i<=10 ; i++) {
		temp1[i] = UART1_DR_R;
		}
		rx_flag=1;
		UART1_ICR_R = UART_ICR_RTIC;        // acknowledge receiver time out
    }
}


//------------UART_InChar------------
// Wait for new serial port input
// Input: none
// Output: ASCII code for key typed
// Consumer consume the data until buffer is EMPTY.
unsigned char UART_InChar(void){
  while((UART1_FR_R&UART_FR_RXFE) != 0);
  return((unsigned char)(UART1_DR_R&0xFF));
}

//------------UART_InCharNonBlocking------------
// Get oldest serial port input and return immediately
// if there is no data.
// Input: none
// Output: ASCII code for key typed or 0 if no character
unsigned char UART_InCharNonBlocking(void){
  if((UART1_FR_R&UART_FR_RXFE) == 0){
    return((unsigned char)(UART1_DR_R&0xFF));
  } else{
    return 0;
  }
}

//------------UART_OutChar------------
// Output 8-bit to serial port
// Input: letter is an 8-bit ASCII character to be transferred
// Output: none
// producer produce the data until buffer is FULL.
void UART_OutChar(unsigned char data){
  while((UART1_FR_R&UART_FR_TXFF) != 0);
  UART1_DR_R = data;
}


void UART_OutString(char *pt){
  while(*pt){
    UART_OutChar(*pt);
    pt++;
  }
}

//-----------------------UART_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void UART_OutUDec(unsigned long n){
// This function uses recursion to convert decimal number
//   of unspecified length as an ASCII string
  if(n >= 10){
    UART_OutUDec(n/10);
    n = n%10;
  }
  UART_OutChar(n+'0'); /* n is between 0 and 9 */
}

