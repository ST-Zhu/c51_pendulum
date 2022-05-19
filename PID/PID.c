#include <reg52.h>
#define uchar unsigned char
#define uint unsigned int
#define ulint long int
#define CYCLE 40				//调整一次周期（PWM周期）

sbit ENA=P1^0;
sbit IN1=P1^1;
sbit IN2=P1^2;

struct PID_Value
{
    ulint liEkVal[3];
    uchar uEkFlag[3];
    uchar uKP_Coe;
    uchar uKI_Coe;
    uchar uKD_Coe;
    uint iPriVal;
    uint iSetVal;
    uint iCurVal;
}PID;
uchar count;					//计数数
uchar PWM;						//所占空间比

void PID_Operation(uchar BIG,uchar LITTLE)//PID运算
{
    ulint Temp[3] = {0};
    ulint PostSum = 0;
    ulint NegSum = 0;
		
        if(BIG - LITTLE > 10)
            PID.iPriVal = CYCLE/4;
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
                if(Temp[0] < CYCLE/4 )
                    PID.iPriVal = (uint)Temp[0];
                else PID.iPriVal = CYCLE/4;
            }
            else;
        }
}

void PID_Output(void)
{
	if(PID.iSetVal>PID.iCurVal)//正转
	{
		PID_Operation(PID.iSetVal,PID.iCurVal);
		IN1=0;
		IN2=1;
	}
	if(PID.iSetVal<PID.iCurVal)//反转
	{
		PID_Operation(PID.iCurVal,PID.iSetVal);
		IN1=1;
		IN2=0;
	}
	PWM=PID.iPriVal;
}

void InitTimer0()
{
	TMOD=0x01;
	TH0=(65536-100)/256;
	TL0=(65536-100)%256;
	EA=1;
	ET0=1;
	TR0=1;		//开定时器0
}

void InitCar()
{
	ENA=1;
}

void Init()
{
	InitTimer0();
	InitCar();
}
 
void main()
{
	Init();
	while(1)
	{
		PID_Output();
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