/*
 * @Author: wushengran
 * @Date: 2024-09-18 09:17:25
 * @Description: 
 * 
 * Copyright (c) 2024 by atguigu, All Rights Reserved. 
 */
#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

// 宏定义LED灯
#define LED_1 GPIO_ODR_ODR0
#define LED_2 GPIO_ODR_ODR1
#define LED_3 GPIO_ODR_ODR8

// 初始化
void LED_Init(void);

// 开关LED灯
void LED_On(uint16_t led);
void LED_Off(uint16_t led);

// 翻转LED灯状态
void LED_Toggle(uint16_t led);

// 控制所有LED灯的开关
void LED_OnAll(uint16_t leds[], uint8_t size);
void LED_OffAll(uint16_t leds[], uint8_t size);

#endif
