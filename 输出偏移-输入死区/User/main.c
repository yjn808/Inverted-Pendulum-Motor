#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "Key.h"
#include "RP.h"
#include "Motor.h"
#include "Encoder.h"
#include "Serial.h"
#include "Math.h"



uint8_t KeyNum;



//PID电机控速
float Target,Actual,Out;
float Kp=0,Ki=0,Kd=0;
//位置式PID
float Erro0,Erro1,ErroInt;


int main(void)
{
	OLED_Init();
	Key_Init();
	Motor_Init();
	Encoder_Init();
	
	Timer_Init();
	
	OLED_Printf(0, 0, OLED_8X16, "Speed Control");
	OLED_Update();
	RP_Init();
	Serial_Init();
	
	
	
	while (1)
	{
		//测试偏移offset
//		KeyNum = Key_GetNum();
//		if(KeyNum==1)
//		{
//			Out += 1;
//		}
//		if(KeyNum == 2)
//		{
//			Out-=1;
//		}
//		Motor_SetPWM(Out);
//		OLED_Printf(0,16,OLED_8X16,"Out:%+04.0f",Out);
//		OLED_Update();
		
		
		
		KeyNum = Key_GetNum();
//		if (KeyNum == 1)
//		{
//			Target += 10;
//			
//		}
//		if (KeyNum == 2)
//		{
//			Target -= 10;
//			
//		}
//		if (KeyNum == 3)
//		{
//			Target = 0;
//		}
		
		
		
		
		
		Kp=RP_GetValue(1)/4095.0*2;
		Ki=RP_GetValue(2)/4095.0*2;
		Kd=RP_GetValue(3)/4095.0*2;
		
		Target=RP_GetValue(4)/4095.0*300-150;
		
		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", Kp);
		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", Ki);
		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", Kd);
		
		
		OLED_Printf(0, 16, OLED_8X16, "Tar:%+04.0f", Target);
		OLED_Printf(0, 32, OLED_8X16, "Act:%+04.0f", Actual);
		OLED_Printf(0, 48, OLED_8X16, "Out:%+04.0f", Out);
		
		
		OLED_Update();
		
		Serial_Printf("%f,%f,%f\r\n",Target,Actual,Out);
	}
}

void TIM1_UP_IRQHandler(void)
{
	static uint16_t Count;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		Key_Tick();
		
		Count++;
		if(Count>=40)
		{
			Count=0;
			//获取实际值
			Actual=Encoder_Get();
			//获取本次误差和上次误差  +上上次误差
			Erro1=Erro0;
			Erro0=Target-Actual;
			
			
			if(fabs(Erro0)<5)
			{
				Out=0;
				
			}
			else
			{
					//位置式PID
				if(Ki != 0)
				{
					//误差积分（累加）
					ErroInt += Erro0;
				}
				else{
					ErroInt=0;
				}
				
				
				/* 位置式PID计算*/
				Out=Kp*Erro0+Ki*ErroInt+Kd*(Erro0-Erro1);
				
				if(Out>0)
				{
					Out+=6;
				}
				else if(Out<0)
				{
					Out -= 6;
				}
				else{
					Out=0;
				}
			}
			
			

			
			
			//输出限幅
			
			if(Out>100){Out=100;}
			if(Out<-100){Out=-100;}
			
			Motor_SetPWM(Out);
			
		}
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}

}


