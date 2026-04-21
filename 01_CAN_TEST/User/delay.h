/*
 * @Author: wushengran
 * @Date: 2024-09-18 09:13:05
 * @Description: 
 * 
 * Copyright (c) 2024 by atguigu, All Rights Reserved. 
 */
#ifndef __DELAY_H
#define __DELAY_H

#include "stm32f10x.h"

// 定义延时函数
void Delay_us(uint16_t us);
void Delay_ms(uint16_t ms);
void Delay_s(uint16_t s);

#endif
