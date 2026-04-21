/*
 * @Author: wushengran
 * @Date: 2024-09-19 15:17:44
 * @Description: 
 * 
 * Copyright (c) 2024 by atguigu, All Rights Reserved. 
 */
#include "key.h"
#include "delay.h"
#include "led.h"

// 初始化
void KEY_Init(void)
{
    // 1. 开启时钟
    RCC->APB2ENR |= RCC_APB2ENR_IOPFEN;
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    // 2. PF10 工作模式：下拉输入，CNF = 10， MODE = 00
    GPIOF->CRH &= ~GPIO_CRH_MODE10;
    GPIOF->CRH |= GPIO_CRH_CNF10_1;
    GPIOF->CRH &= ~GPIO_CRH_CNF10_0;

    GPIOF->ODR &= ~GPIO_ODR_ODR10;

    // 3. AFIO 引脚复用配置 EXTI10 - PF10
    AFIO->EXTICR[2] |= AFIO_EXTICR3_EXTI10_PF;

    // 4. EXTI配置
    // 4.1 配置上升沿触发
    EXTI->RTSR |= EXTI_RTSR_TR10;
    // 4.2 打开屏蔽位
    EXTI->IMR |= EXTI_IMR_MR10;

    // 5. NVIC配置
    // 5.1 设置优先级组方案
    NVIC_SetPriorityGrouping(3);    // 全部4位都是抢占优先级
    NVIC_SetPriority(EXTI15_10_IRQn, 2);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// 中断处理程序
void EXTI15_10_IRQHandler(void)
{
    // 1. 清除挂起位（PR 置1）
    EXTI->PR |= EXTI_PR_PR10;

    // 2. 防抖延时
    Delay_ms(15);

    // 3. 确认当前PF10依然输入高电平，就翻转LED1
    if ((GPIOF->IDR & GPIO_IDR_IDR10) != 0)
    {
        LED_Toggle(LED_1);
    }
}
