/*
 * @Author: wushengran
 * @Date: 2024-09-18 09:17:17
 * @Description:
 *
 * Copyright (c) 2024 by atguigu, All Rights Reserved.
 */
#include "led.h"

// 初始化
void LED_Init(void)
{
    // 1. 时钟配置，打开GPIOA时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // 2. 工作模式配置，PA0、PA1、PA8 通用推挽输出，CNF = 00，MODE = 11
    GPIOA->CRL |= GPIO_CRL_MODE0;
    GPIOA->CRL &= ~GPIO_CRL_CNF0;
    GPIOA->CRL |= GPIO_CRL_MODE1;
    GPIOA->CRL &= ~GPIO_CRL_CNF1;
    GPIOA->CRH |= GPIO_CRH_MODE8;
    GPIOA->CRH &= ~GPIO_CRH_CNF8;

    // 3. 初始状态所有引脚输出高电平，关灯
    LED_Off(LED_1);
    LED_Off(LED_2);
    LED_Off(LED_3);
}

// 开关LED灯
void LED_On(uint16_t led)
{
    GPIOA->ODR &= ~led;
}
void LED_Off(uint16_t led)
{
    GPIOA->ODR |= led;
}

// 翻转LED灯状态
void LED_Toggle(uint16_t led)
{
    // 根据IDR对应位的值，判断当前LED状态
    if ((GPIOA->IDR & led) == 0)
    {
        LED_Off(led);
    }
    else
    {
        LED_On(led);
    }
}

// 控制所有LED灯的开关
void LED_OnAll(uint16_t leds[], uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        LED_On(leds[i]);
    }
}
void LED_OffAll(uint16_t leds[], uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        LED_Off(leds[i]);
    }
}
