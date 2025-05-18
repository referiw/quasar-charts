#include "delay.h"
#include "sys.h"

//********************************************************************************
// 延时函数
//********************************************************************************
static u8 fac_us = 0;  // 微秒延时倍乘数
static u16 fac_ms = 0; // 毫秒延时倍乘数

// 初始化延迟函数： SYSTICK的时钟固定为HCLK时钟的1/8
// SYSCLK:系统时钟
void delay_init()
{
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); // 选择外部时钟  HCLK/8
	fac_us = SystemCoreClock / 9000000;					  // 为系统时钟的1/9
	fac_ms = (u16)fac_us * 1000;						  // 每个ms需要的systick时钟数
}

// 微秒延时，us为要延时的微秒数.
void delay_us(u32 us)
{
	u32 temp;
	SysTick->LOAD = us * fac_us;			  // 时间加载
	SysTick->VAL = 0x00;					  // 清空计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // 开始倒数
	do
	{
		temp = SysTick->CTRL;
	} while (temp & 0x01 && !(temp & (1 << 16))); // 等待时间到达
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // 关闭计数器
	SysTick->VAL = 0X00;					   // 清空计数器
}
// 毫秒延时，ms为要延时的毫秒数.
// SysTick->LOAD为24位寄存器,所以,最大延时为:ms<=0xffffff*8*1000/SYSCLK
// SYSCLK单位为Hz,ms单位为ms, 对72M条件下,ms<=1864
void delay_ms(u16 ms)
{
	u32 temp;
	SysTick->LOAD = (u32)ms * fac_ms;		  // 时间加载(SysTick->LOAD为24bit)
	SysTick->VAL = 0x00;					  // 清空计数器
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // 开始倒数
	do
	{
		temp = SysTick->CTRL;
	} while (temp & 0x01 && !(temp & (1 << 16))); // 等待时间到达
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // 关闭计数器
	SysTick->VAL = 0X00;					   // 清空计数器
}
