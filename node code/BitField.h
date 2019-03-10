#include<stdlib.h>

#ifndef _Bit_Field
	#define	_Bit_Field
typedef enum{
	lock		=		0x4C4F434B,
	Bit0		=		0x00000001,
	Bit1		=		0x00000002,
	Bit2		=		0x00000004,
	Bit3		=		0x00000008,
	Bit4		=		0x00000010,
	Bit5		=		0x00000020,
	Bit6		=		0x00000040,
	Bit7		=		0x00000080,
	Bit8		=		0x00000100,
	Bit9		=		0x00000200,
	Bit10		=		0x00000400,
	Bit11		=		0x00000800,
	Bit12		=		0x00001000,
	Bit13		=		0x00002000,
	Bit14		=		0x00004000,
	Bit15		=		0x00008000,
	Bit16		=		0x00010000,
	Bit17		=		0x00020000,
	Bit18		=		0x00040000,
	Bit19		=		0x00080000,
	Bit20		=		0x00100000,
	Bit21		=		0x00200000,
	Bit22		=		0x00400000,
	Bit23		=		0x00800000,
	Bit24		=		0x01000000,
	Bit25		=		0x02000000,
	Bit26		=		0x04000000,
	Bit27		=		0x08000000,
	Bit28		=		0x10000000,
	Bit29		=		0x20000000,
	Bit30		=		0x40000000,
	Bit31		=		0x80000000
} BIT;

typedef enum{
	RXI			=		0x00000010,
	TXI			=		0X00000020,
	RTI			=		0X00000040,
	FEI			=		0X00000080,
	PEI			=		0X00000100,
	BEI			=		0X00000200,
	OEI			=		0X00000400,
	N_BtI		=		0X00001000
} UART;

typedef enum{
	SC1		=		0x55,
	SC2		=		0xAA,
	dv1		=		0x01,
	dv2		=		0x00
} FingerPrint;

#endif
