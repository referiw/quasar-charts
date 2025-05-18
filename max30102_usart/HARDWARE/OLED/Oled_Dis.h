#ifndef __OLED2_H
#define __OLED2_H 
#include "sys.h"
#include "stdlib.h"	

//-----------------IIC OLED端口定义---------------- 
// IIC OLED<->STM32:
// VCC<->3.3V
// GND<->GND
// SCL<->PB12
// SDA<->PB13
#define OLED_SCL_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_13)//SCL
#define OLED_SCL_Set() GPIO_SetBits(GPIOB,GPIO_Pin_13)
#define OLED_SDA_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_12)//DIN
#define OLED_SDA_Set() GPIO_SetBits(GPIOB,GPIO_Pin_12)
#define OLED_WR_Byte(dat,mode) OLED_IIC_WriteData(dat,mode)

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据

void OLED_SPI_WriteData(u8 dat,u8 mode);
void OLED_IIC_WriteData(u8 dat,u8 mode);
void OLED_ColorTurn(u8 i);
void OLED_DisplayTurn(u8 i);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_WaitAck(void);
void Send_Byte(u8 dat);
//void OLED_WR_Byte(u8 dat,u8 mode);
void OLED_Set_Pos(u8 x,u8 y);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_Clear(void);
void OLED_ShowChar(u8 x,u8 y,u8 Char);
void OLED_ShowString(u8 x,u8 y,u8 ch[]);
u32 OLED_Pow(u8 m,u8 n);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);
void OLED_ShowChinese(u8 x,u8 y,u8 NO);
void OLED_P32x64Str(u8 x, u8 y,u8 dat);
void OLED_wave(u8 Wave_sum);
void OLED_Init(void);
#endif

