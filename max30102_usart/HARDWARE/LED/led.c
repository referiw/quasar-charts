#include "led.h"

//////////////////////////////////////////////////////////////////////////////////	 
void LED_Init(void)
{
 
// GPIO_InitTypeDef  GPIO_InitStructure;
// 	
// RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB
//	
// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				 // �˿�����
// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
// GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB-11
// GPIO_SetBits(GPIOB,GPIO_Pin_11);						 //PB.11 �����
}
 
void BEEP_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				 // �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB-11
 GPIO_SetBits(GPIOB,GPIO_Pin_11);						 //PB.11 �����
}

void BEEP_on(u8 status) //0-�ر�,1-����
{
	if(status)GPIO_ResetBits(GPIOB,GPIO_Pin_11);
	else GPIO_SetBits(GPIOB,GPIO_Pin_11);
}

//void BEEP_off(u8 status)
//{
//	GPIO_SetBits(GPIOB,GPIO_Pin_11);
//}


