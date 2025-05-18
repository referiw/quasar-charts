#include "led.h"

//////////////////////////////////////////////////////////////////////////////////	 
void LED_Init(void)
{
 
// GPIO_InitTypeDef  GPIO_InitStructure;
// 	
// RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB
//	
// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				 // 端口配置
// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
// GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB-11
// GPIO_SetBits(GPIOB,GPIO_Pin_11);						 //PB.11 输出高
}
 
void BEEP_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				 // 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB-11
 GPIO_SetBits(GPIOB,GPIO_Pin_11);						 //PB.11 输出高
}

void BEEP_on(u8 status) //0-关闭,1-开启
{
	if(status)GPIO_ResetBits(GPIOB,GPIO_Pin_11);
	else GPIO_SetBits(GPIOB,GPIO_Pin_11);
}

//void BEEP_off(u8 status)
//{
//	GPIO_SetBits(GPIOB,GPIO_Pin_11);
//}


