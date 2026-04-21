/*
 * @Author: wushengran
 * @Date: 2024-09-20 16:23:02
 * @Description:
 *
 * Copyright (c) 2024 by atguigu, All Rights Reserved.
 */
#include "usart.h"

// 初始化
void USART_Init(void)
{
    // 1. 开启时钟
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;

    // 2. GPIO 工作模式
    // 2.1 PA9 - TX，复用推挽输出，CNF = 10，MODE = 11
    GPIOA->CRH |= GPIO_CRH_MODE9;
    GPIOA->CRH |= GPIO_CRH_CNF9_1;
    GPIOA->CRH &= ~GPIO_CRH_CNF9_0;
    // 2.2 PA10 - RX，浮空输入，CNF = 01，MODE = 00
    GPIOA->CRH &= ~GPIO_CRH_MODE10;
    GPIOA->CRH &= ~GPIO_CRH_CNF10_1;
    GPIOA->CRH |= GPIO_CRH_CNF10_0;

    // 3. 串口模块配置
    // 3.1 设置波特率 115200
    USART1->BRR = 0x271;

    // 3.2 使能串口和收发模块
    USART1->CR1 |= USART_CR1_UE;
    USART1->CR1 |= (USART_CR1_TE | USART_CR1_RE);

    // 3.3 配置数据帧的格式
    USART1->CR1 &= ~USART_CR1_M;    // 长度为 8 位
    USART1->CR1 &= ~USART_CR1_PCE;  // 不使用校验位
    USART1->CR2 &= ~USART_CR2_STOP; // 1 位停止位
}

// 发送一个字符
void USART_SendChar(uint8_t ch)
{
    // 判断 TDR 是否为空，必须等待 TDR 为空才能继续发送
    while ((USART1->SR & USART_SR_TXE) == 0)
    {
    }

    // 将要发送的数据写入TDR
    USART1->DR = ch;
}

// 接收一个字符
uint8_t USART_ReceiveChar(void)
{
    // 判断 RDR 是否非空，必须等待 RDR 有数据才能读取出来
    while ((USART1->SR & USART_SR_RXNE) == 0)
    {
    }

    // 读取接收到的数据，返回
    return USART1->DR;
}

// 发送字符串
void USART_SendString(uint8_t *str, uint8_t size)
{
    for (uint8_t i = 0; i < size; i++)
    {
        USART_SendChar(str[i]);
    }
}

// 接收字符串
void USART_ReceiveString(uint8_t buffer[], uint8_t *size)
{
    // 定义变量，保存当前接收到的字符个数
    uint8_t i = 0;

    // 不停地接收字符，直到检测到空闲帧

    // 错误写法：
    // while ( (USART1->SR & USART_SR_IDLE) == 0 )
    // {
    // 	buffer[i] = USART_ReceiveChar();
    // 	i++;
    // }

    // 正确写法：
    // 外层循环：不停读取下一个字符
    while (1)
    {
        // 内层循环：判断当前数据帧是否结束
        while ((USART1->SR & USART_SR_RXNE) == 0)
        {
            // 一旦已经检测到空闲帧，就立刻退出
            if (USART1->SR & USART_SR_IDLE)
            {
                *size = i;
                USART1->DR;
                return;
            }
        }
        buffer[i] = USART1->DR;
        i++;
    }
}

// 重写fputc函数
int fputc(int ch, FILE * file)
{
    USART_SendChar(ch);
    return ch;
}
