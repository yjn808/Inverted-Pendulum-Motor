/*
 * @Author: wushengran
 * @Date: 2024-09-13 16:29:39
 * @Description:
 *
 * Copyright (c) 2024 by atguigu, All Rights Reserved.
 */

#include "usart.h"
#include "delay.h"
#include "can.h"
#include <string.h>

int main(void)
{
	// 初始化
	USART_Init();
	CAN_Init();

	printf("Hello world!\n");

	//1.发送三个报文
	uint16_t id1=0x066;
	uint8_t * data = "abc";
	
	//定义发送缓冲区
	uint8_t buffer[10];
	uint32_t i =0;
	



	while (1)
	{
		sprintf((char *)buffer,"%s %d",data,++i);
		
		
		CAN_SendMessage(id1,data,strlen((char *)data));
		

		Delay_ms(100);
	}
}

