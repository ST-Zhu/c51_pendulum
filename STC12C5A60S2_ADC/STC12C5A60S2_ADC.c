#include "reg52.h"
#include "intrins.h"
typedef unsigned char uchar;
typedef unsigned int uint;

sfr ADC_CONTR=0xBC;
sfr ADC_RES=0xBD;
sfr ADC_LOW2=0xBE;
sfr P1ASF=0x9D;

#define ADC_POWER 0x80
#define ADC_FLAG 0x10
#define ADC_START 0x08
#define ADC_SPEEDLL 0x00
#define ADC_SPEEDL 0x20
#define ADC_SPEEDH 0x40
#define ADC_SPEEDHH 0x60
sbit Angle=P1^0;

void InitUart();
void InitADC();
void SendData(uchar dat);
uchar GetADCResult(uchar ch);
void Delay_ADC(uint n);
void ShowResult(uchar ch);
void delay(uint z);

void main()
{
	InitUart();
	InitADC();
	while(1)
	{
		ShowResult(0);
		delay(100);
	}
}

void ShowResult(uchar ch)
{
	SendData(ch);
	SendData(GetADCResult(ch));
}

uchar GetADCResult(uchar ch)
{
	ADC_CONTR=ADC_POWER|ADC_SPEEDLL|ch|ADC_START;
	_nop_();
	_nop_();
	_nop_();
	_nop_();
	while(!(ADC_CONTR&ADC_FLAG));
	ADC_CONTR&=~ADC_FLAG;
	
	return ADC_RES;
}

void InitUart()
{
	SCON=0x5a;
	TMOD=0x20;
	TH1=0xfd;
	TL1=0xfd;
	TR1=1;
}

void InitADC()
{
	P1ASF=0x1c;
	Angle=1;
	ADC_RES=0;
	ADC_CONTR=ADC_POWER|ADC_SPEEDLL;
	Delay_ADC(2);
}

void SendData(uchar dat)
{
	while(!TI);
	TI=0;
	SBUF=dat;
}

void Delay_ADC(uint n)
{
	uint x;
	while(n--)
	{
		x=5000;
		while(x--);
	}
}

void delay(uint z)
{
	uint x,y;
	for(x=z;x>0;x--)
		for(y=10000;y>0;y--);
}
