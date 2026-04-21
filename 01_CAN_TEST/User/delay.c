/*
 * @Author: wushengran
 * @Date: 2024-09-18 09:13:00
 * @Description: 
 * 
 * Copyright (c) 2024 by atguigu, All Rights Reserved. 
 */
#include "delay.h"

// 微秒级延时函数，延迟us微秒
void Delay_us(uint16_t us)
{
	// 设置系统定时器的初始计数值
	SysTick->LOAD = 72 * us;

	// 计数器值清零
	SysTick->VAL = 0;

	// 配置系统定时器
	SysTick->CTRL = 0x05;

	// 轮询等待计数值变为0，COUNTFLAG = 1
	while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG))
	{
	}

	// 关闭定时器
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE;
}

void Delay_ms(uint16_t ms)
{
	while (ms--)
	{
		Delay_us(1000);
	}
}
void Delay_s(uint16_t s)
{
	while (s--)
	{
		Delay_ms(1000);
	}
}
