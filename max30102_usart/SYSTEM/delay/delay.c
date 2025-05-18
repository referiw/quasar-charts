#include "delay.h"
#include "sys.h"

//********************************************************************************
// ��ʱ����
//********************************************************************************
static u8 fac_us = 0;  // ΢����ʱ������
static u16 fac_ms = 0; // ������ʱ������

// ��ʼ���ӳٺ����� SYSTICK��ʱ�ӹ̶�ΪHCLKʱ�ӵ�1/8
// SYSCLK:ϵͳʱ��
void delay_init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); // ѡ���ⲿʱ��  HCLK/8
	fac_us = SystemCoreClock / 9000000;					  // Ϊϵͳʱ�ӵ�1/9
	fac_ms = (u16)fac_us * 1000;						  // ÿ��ms��Ҫ��systickʱ����
}

// ΢����ʱ��usΪҪ��ʱ��΢����.
void delay_us(u32 us)
{
	u32 temp;
	SysTick->LOAD = us * fac_us;			  // ʱ�����
	SysTick->VAL = 0x00;					  // ��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // ��ʼ����
	do
	{
		temp = SysTick->CTRL;
	} while (temp & 0x01 && !(temp & (1 << 16))); // �ȴ�ʱ�䵽��
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // �رռ�����
	SysTick->VAL = 0X00;					   // ��ռ�����
}
// ������ʱ��msΪҪ��ʱ�ĺ�����.
// SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:ms<=0xffffff*8*1000/SYSCLK
// SYSCLK��λΪHz,ms��λΪms, ��72M������,ms<=1864
void delay_ms(u16 ms)
{
	u32 temp;
	SysTick->LOAD = (u32)ms * fac_ms;		  // ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL = 0x00;					  // ��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // ��ʼ����
	do
	{
		temp = SysTick->CTRL;
	} while (temp & 0x01 && !(temp & (1 << 16))); // �ȴ�ʱ�䵽��
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // �رռ�����
	SysTick->VAL = 0X00;					   // ��ռ�����
}
