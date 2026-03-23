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




uint8_t KeyNum;
int16_t Speed,Location;


//内环位置PID
float InnerTarget,InnerActual,InnerOut;
float InnerKp=0.3,InnerKi=0.3,InnerKd=0;
float InnerError0,InnerError1,InnerErrorInt;

//外环速度PID
float OuterTarget,OuterActual,OuterOut;
float OuterKp=0,OuterKi=0,OuterKd=0;
float OuterError0,OuterError1,OuterErrorInt;


int main(void)
{
	OLED_Init();
	Key_Init();
	Motor_Init();
	Encoder_Init();
	
	Timer_Init();
	
	OLED_Printf(0, 0, OLED_8X16, "2*PID Control");
	OLED_Update();
	RP_Init();
	Serial_Init();
	
	
	
	while (1)
	{
//		KeyNum = Key_GetNum();
////		if (KeyNum == 1)
////		{
////			InnerTarget += 10;
////			
////		}
////		if (KeyNum == 2)
////		{
////			InnerTarget -= 10;
////			
////		}
////		if (KeyNum == 3)
////		{
////			InnerTarget = 0;
////		}
		
		
		
		
		
//		InnerKp=RP_GetValue(1)/4095.0*2;
//		InnerKi=RP_GetValue(2)/4095.0*2;
//		InnerKd=RP_GetValue(3)/4095.0*2;
//		
//		InnerTarget=RP_GetValue(4)/4095.0*300-150;
//		
//		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", InnerKp);
//		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", InnerKi);
//		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", InnerKd);
//		
//		
//		OLED_Printf(0, 16, OLED_8X16, "Tar:%+04.0f", InnerTarget);
//		OLED_Printf(0, 32, OLED_8X16, "Act:%+04.0f", InnerActual);
//		OLED_Printf(0, 48, OLED_8X16, "Out:%+04.0f", InnerOut);
//		
//		
//		OLED_Update();
//		
//		Serial_Printf("%f,%f,%f\r\n",InnerTarget,InnerActual,InnerOut);
		
		
		
		
		OuterKp=RP_GetValue(1)/4095.0*2;
		OuterKi=RP_GetValue(2)/4095.0*2;
		OuterKd=RP_GetValue(3)/4095.0*2;
		
		OuterTarget=RP_GetValue(4)/4095.0*816-408;
		
		OLED_Printf(0, 16, OLED_8X16, "Kp:%4.2f", OuterKp);
		OLED_Printf(0, 32, OLED_8X16, "Ki:%4.2f", OuterKi);
		OLED_Printf(0, 48, OLED_8X16, "Kd:%4.2f", OuterKd);
		
		
		OLED_Printf(0, 16, OLED_8X16, "Tar:%+04.0f", OuterTarget);
		OLED_Printf(0, 32, OLED_8X16, "Act:%+04.0f", OuterActual);
		OLED_Printf(0, 48, OLED_8X16, "Out:%+04.0f", OuterOut);
		
		
		OLED_Update();
		
		Serial_Printf("%f,%f,%f\r\n",OuterTarget,OuterActual,OuterOut);
	}
}

void TIM1_UP_IRQHandler(void)
{
	static uint16_t Count1,Count2;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
	{
		Key_Tick();
		
		Count1++;
		if(Count1>=40)
		{
			Count1=0;
			
			Speed =Encoder_Get();
			Location += Speed;
			
			//获取实际值
			InnerActual=Speed;
			//获取本次误差和上次误差  
			InnerError1=InnerError0;
			InnerError0=InnerTarget-InnerActual;
			
			//位置式PID
			if(InnerKi != 0)
			{
				//误差积分（累加）
				InnerErrorInt += InnerError0;
			}
			else{
				InnerErrorInt=0;
			}
			
			
			/* 位置式PID计算*/
			InnerOut=InnerKp*InnerError0+InnerKi*InnerErrorInt+InnerKd*(InnerError0-InnerError1);



			
			//输出限幅
			
			if(InnerOut>100){InnerOut=100;}
			if(InnerOut<-100){InnerOut=-100;}
			
			Motor_SetPWM(InnerOut);
			
		}
		
		
		Count2++;
		if(Count2>=40)
		{
			Count2=0;
			//获取实际值
			OuterActual=Location;
			//获取本次误差和上次误差  
			OuterError1=OuterError0;
			OuterError0=OuterTarget-OuterActual;
			
			//位置式PID
			if(OuterKi != 0)
			{
				//误差积分（累加）
				OuterErrorInt += OuterError0;
			}
			else{
				OuterErrorInt=0;
			}
			
			
			/* 位置式PID计算*/
			OuterOut=OuterKp*OuterError0+OuterKi*OuterErrorInt+OuterKd*(OuterError0-OuterError1);



			
			//输出限幅  改变旋转速度！限制输出速度
			
			if(OuterOut>150){OuterOut=150;}
			if(OuterOut<-150){OuterOut=-150;}
			
			InnerTarget=OuterOut;
			
		}
		
		
		
		
		
		
		
		
		
		
		
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}



