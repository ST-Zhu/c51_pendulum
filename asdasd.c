#include <reg52.h>
#include "intrins.h"
#define uint unsigned int
#define uchar unsigned char
#define ulint long int
#define CYCLE 20				//调整一次周期（PWM周期）

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

uchar count;					//计数数
uchar PWM;						//所占空间比

sbit KEY1=P0^0;
sbit KEY2=P0^1;
sbit KEY3=P0^2;
sbit ENA=P2^0;
sbit IN1=P2^1;
sbit IN2=P2^2;
sbit Angle=P1^0;

struct PID_Value
{
    ulint liEkVal[3];
    uchar uEkFlag[3];
    float uKP_Coe;
    float uKI_Coe;
    float uKD_Coe;
    uint iPriVal;
    uint iSetVal;
    uint iCurVal;
}PID;

void InitUart();
void InitADC();
void SendData(uchar dat);
uchar GetADCResult(uchar ch);
void Delay_ADC(uint n);
void ShowResult(uchar ch);
void delay(uint z);

void ShowResult(uchar ch)
{
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
	TMOD|=0x20;
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
		for(y=1000;y>0;y--);
}

void PID_Operation(uchar BIG,uchar LITTLE)//PID运算
{
    ulint Temp[3] = {0};
    ulint PostSum = 0;
    ulint NegSum = 0;
		
        if(BIG - LITTLE > 20)
            PID.iPriVal = CYCLE;
        else
        {
            Temp[0] = BIG - LITTLE;
            PID.uEkFlag[1] = 0;
            PID.liEkVal[2] = PID.liEkVal[1];
            PID.liEkVal[1] = PID.liEkVal[0];
            PID.liEkVal[0] = Temp[0];
					
            if(PID.liEkVal[0] > PID.liEkVal[1])
            {
                Temp[0] = PID.liEkVal[0] - PID.liEkVal[1];
                PID.uEkFlag[0] = 0;
            }                                       
            else
            {
                Temp[0] = PID.liEkVal[1] - PID.liEkVal[0];
                PID.uEkFlag[0] = 1;
            }
						
            Temp[2] = PID.liEkVal[1] * 2;
            if((PID.liEkVal[0] + PID.liEkVal[2]) > Temp[2])
            {
                Temp[2] = (PID.liEkVal[0] + PID.liEkVal[2]) - Temp[2];
                PID.uEkFlag[2]=0;
            }                                               
            else
            {
                Temp[2] = Temp[2] - (PID.liEkVal[0] + PID.liEkVal[2]); 
                PID.uEkFlag[2] = 1;
            }
						
            Temp[0] = (ulint)PID.uKP_Coe * Temp[0];
            Temp[1] = (ulint)PID.uKI_Coe * PID.liEkVal[0];
            Temp[2] = (ulint)PID.uKD_Coe * Temp[2];
						
            if(PID.uEkFlag[0] == 0)
                PostSum += Temp[0];
            else                                            
                NegSum += Temp[0];

            if(PID.uEkFlag[1] == 0)     
                PostSum += Temp[1];
            else;

            if(PID.uEkFlag[2]==0)
                PostSum += Temp[2];
            else
                NegSum += Temp[2];
                    
            PostSum += (ulint)PID.iPriVal;
						
            if(PostSum > NegSum)
            { 
                Temp[0] = PostSum - NegSum;
                if(Temp[0] < CYCLE )
                    PID.iPriVal = (uint)Temp[0];
                else PID.iPriVal = CYCLE;
            }
            else;
        }
}

void PID_Output()
{
	PID.iCurVal=GetADCResult(0);
	if(PID.iSetVal>=PID.iCurVal)//正转
	{
		PID_Operation(PID.iSetVal,PID.iCurVal);
		IN1=1;
		IN2=0;
	}
	if(PID.iSetVal<PID.iCurVal)//反转
	{
		PID_Operation(PID.iCurVal,PID.iSetVal);
		IN1=0;
		IN2=1;
	}
	PWM=PID.iPriVal;
}

void InitCar()
{
	ENA=1;
}

void InitPID()//PID参数赋值
{
	PID.uKP_Coe=3.6;
	PID.uKI_Coe=1.61;
	PID.uKD_Coe=26.5;
}

void InitTimer0()
{
	TMOD|=0x01;
	TH0=(65536-100)/256;
	TL0=(65536-100)%256;
	EA=1;
	ET0=1;
	TR0=1;		//开定时器0
}

void Init()
{
	InitCar();
	InitPID();
	InitADC();
	InitUart();
	InitTimer0();
}

void main()
{
	uchar i;
	Init();
	while(1)
	{			
		if(KEY1==0)//功能一
		{
			delay(30);
			if(KEY1==0)
			{
				while(KEY1==0);	
				while(KEY1!=0)
				{
					IN1=0;
					IN2=1;
					PWM=12;
					delay(280);
					IN1=1;
					IN2=0;
					PWM=12;
					delay(280);
				}
			}
		}
		if(KEY2==0)//功能二
		{
			delay(30);
			if(KEY2==0)
			{
				while(KEY2==0);
				while(KEY2!=0)
				{	
					IN1=0;
					IN2=1;
					PWM=18;
					delay(300);
					IN1=1;
					IN2=0;
					PWM=18;
					delay(300);
				}
			}
		}
		if(KEY3==0)//功能三
		{
			delay(30);
			if(KEY3==0)
			{
				while(KEY3==0);
				while(KEY3!=0)
				{
					PID.iSetVal=0xAB;
					PID_Output();
				}
			}
		}
		IN1=0;
		IN2=0;
		PWM=0;
		delay(1000);
	}
}

void timer0() interrupt 1 //定时器0中断（100us）
{
	count++;
	if(count<=PWM)
		ENA=1;
  else
		ENA=0;
	if(count==CYCLE)
	{
		ENA=~ENA;
		count=0;
	}
	TH0=(65536-100)/256;
	TL0=(65536-100)%256;
}//PWM调速
