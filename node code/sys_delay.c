/*#include "TM4C123GH6PM.h"
#include "BitField.h"

#define mS 80000UL
#define uS 80UL

void Init_SysTick(){
	SysTick->CTRL			&=			0;
	SysTick->CTRL			&=			Bit0;														//Disable SysTickTimer
	SCB->SHP[3] 			&=			(Bit29 | Bit30 | Bit31);
	//SCB->SHP[3] 			|=			Bit30;													//Set priority level to 0 in SYSPRI3 register
	SysTick->CTRL			|=			(Bit2);
	//SCB->SCR					&=			(Bit2 | Bit4);
}

void _delay_ms(unsigned int delay){
	SysTick->LOAD			 =			(mS * delay) - 1;
	SysTick->VAL			 =			0;
	SysTick->CTRL			|=			(Bit0);
	
	while((SysTick->CTRL & Bit16) == 0);
	//WaitForInterrupt();
	
	SysTick->CTRL			&=			Bit0;
}

void _delay_us(unsigned int delay){
	SysTick->LOAD			 =			(uS * delay) - 1;
	SysTick->VAL			 =			0;
	SysTick->CTRL			|=			(Bit0 | Bit1);
	
	while((SysTick->CTRL & Bit16) == 0);
	//WaitForInterrupt();
	
	SysTick->CTRL			&=			Bit0;
}

void _delay_100ms(unsigned int delay){
	int i;
	for(i = 0; i < delay; i++) _delay_ms(100);
}

void _delay_100us(unsigned int delay){
	int i;
	for(i = 0; i < delay; i++) _delay_us(100);
}

*/

#define NVIC_ST_CTRL_R          (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R        (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R       (*((volatile unsigned long *)0xE000E018))
#define NVIC_ST_CTRL_COUNT      0x00010000  // Count flag
#define NVIC_ST_CTRL_CLK_SRC    0x00000004  // Clock Source
#define NVIC_ST_CTRL_INTEN      0x00000002  // Interrupt enable
#define NVIC_ST_CTRL_ENABLE     0x00000001  // Counter mode
// Maximum Delay: 2^24*20nsec = 320ms		
//SysTick_Wait(0x00FFFFFF); // approximately 320ms
//#define NVIC_ST_RELOAD_M        0x00FFFFFF  // Counter load value - Max Delay 320msec
// Minimum Delay ; 1sec - 80M ticks ; 1tick - 12.5nsec;
#define NVIC_ST_RELOAD_M        80000 // Counter load value - Max Delay 320msec

// Minimum Delay ; 1sec - 50M ticks ; 1tick - 20nsec;
//#define NVIC_ST_RELOAD_M        0x00000002  // Counter load value - 40 nsec

//(5000000)base10 ticks (or) (0x4c4b40)base16 ticks - 100msec 
//#define NVIC_ST_RELOAD_M        0x004C4B40  // 100msec
// SysTick_Wait(0x004C4B40); // approximately 100ms
//SysTick_Wait(5000000); // approximately 100m

//(500000)base10 ticks - 10msec 
//#define NVIC_ST_RELOAD_M        500000  // 10msec
//  SysTick_Wait(500000); // approximately 10ms


//  SysTick_Wait10ms(100);    // approximately 1s

// Initialize SysTick with busy wait running at bus clock.

void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
  NVIC_ST_RELOAD_R = NVIC_ST_RELOAD_M;  // maximum reload value
  NVIC_ST_CURRENT_R = 0;                // any write to current clears it
                                        // enable SysTick with core clock
  NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE+NVIC_ST_CTRL_CLK_SRC;
}
// Time delay using busy wait.
// The delay parameter is in units of the core clock. (units of 20 nsec for 50 MHz clock)
#define ms 80000
#define us 80
void _delay_ms(unsigned long delay){
  NVIC_ST_RELOAD_R = (ms*delay)-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}
void _delay_us(unsigned long delay){
  NVIC_ST_RELOAD_R = (us*delay)-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}

// Time delay using busy wait.
// This assumes 50 MHz system clock.
void SysTick_Wait10ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    _delay_ms(500000);  // wait 10ms (assumes 50 MHz clock)
  }
}


