#include "Oled_Dis.h"
#include "stdlib.h"
#include "oled_font.h"
#include "delay.h"

// 延时
void IIC_delay(void)
{
	u8 t = 10;
	while (t--)
		;
}
// 起始信号
void I2C_Start(void)
{
	OLED_SDA_Set();
	OLED_SCL_Set();
	IIC_delay();
	OLED_SDA_Clr();
	IIC_delay();
	OLED_SCL_Clr();
	IIC_delay();
}
// 结束信号
void I2C_Stop(void)
{
	OLED_SDA_Clr();
	OLED_SCL_Set();
	IIC_delay();
	OLED_SDA_Set();
}
// 等待信号响应
void I2C_WaitAck(void) // 测数据信号的电平
{
	OLED_SDA_Set();
	IIC_delay();
	OLED_SCL_Set();
	IIC_delay();
	OLED_SCL_Clr();
	IIC_delay();
}
// 写入一个字节
void Send_Byte(u8 dat)
{
	u8 i;
	for (i = 0; i < 8; i++)
	{
		if (dat & 0x80)
		{ // 将dat的8位从最高位依次写入
			OLED_SDA_Set();
		}
		else
		{
			OLED_SDA_Clr();
		}
		IIC_delay();
		OLED_SCL_Set();
		IIC_delay();
		OLED_SCL_Clr(); // 将时钟信号设置为低电平
		dat <<= 1;
	}
}
void OLED_IIC_WriteData(u8 dat, u8 mode)
{
	I2C_Start();
	Send_Byte(0x78);
	I2C_WaitAck();
	if (mode)
	{
		Send_Byte(0x40);
	}
	else
	{
		Send_Byte(0x00);
	}
	I2C_WaitAck();
	Send_Byte(dat);
	I2C_WaitAck();
	I2C_Stop();
}

// 反显函数
void OLED_ColorTurn(u8 i)
{
	if (i == 0)
	{
		OLED_WR_Byte(0xA6, OLED_CMD);
	} // 正常显示
	if (i == 1)
	{
		OLED_WR_Byte(0xA7, OLED_CMD);
	} // 反色显示
}
// 屏幕旋转180度
void OLED_DisplayTurn(u8 i)
{
	if (i == 0)
	{
		OLED_WR_Byte(0xC8, OLED_CMD); // 正常显示
		OLED_WR_Byte(0xA1, OLED_CMD);
	}
	if (i == 1)
	{
		OLED_WR_Byte(0xC0, OLED_CMD); // 反转显示
		OLED_WR_Byte(0xA0, OLED_CMD);
	}
}
// 发送一个字节
// mode:数据/命令标志 0,表示命令;1,表示数据;
// void OLED_WR_Byte(u8 dat,u8 mode){}
void OLED_Set_Pos(u8 x, u8 y)
{
	OLED_WR_Byte(0xb0 + y, OLED_CMD);
	OLED_WR_Byte((x & 0x0f), OLED_CMD);
	OLED_WR_Byte((x & 0xf0) >> 4 | 0x10, OLED_CMD);
}
// 开启OLED显示
void OLED_ON(void)
{
	OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵使能
	OLED_WR_Byte(0x14, OLED_CMD); // 开启电荷泵
	OLED_WR_Byte(0xAF, OLED_CMD); // 点亮屏幕
}
// 关闭OLED显示
void OLED_OFF(void)
{
	OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵使能
	OLED_WR_Byte(0x10, OLED_CMD); // 关闭电荷泵
	OLED_WR_Byte(0xAE, OLED_CMD); // 关闭屏幕
}
// 清屏函数
void OLED_Clear(void)
{
	u8 i, n;
	for (i = 0; i < 8; i++)
	{
		OLED_WR_Byte(0xb0 + i, OLED_CMD);
		OLED_WR_Byte(0x00, OLED_CMD);
		OLED_WR_Byte(0x10, OLED_CMD);
		for (n = 0; n < 128; n++)
		{
			OLED_WR_Byte(0, OLED_DATA);
		}
	}
}
// 在指定位置显示一个字符,包括部分字符
// x:0~127
// y:0~63
// size1:选择字体 6x8/6x12/8x16/12x24
void OLED_ShowChar(u8 x, u8 y, u8 Char)
{
	u8 c, i;
	c = Char - ' ';
	if (x > 128)
	{
		x = 0;
		y += 2;
	}
	OLED_Set_Pos(x, y);
	for (i = 0; i < 8; i++)
	{
		OLED_WR_Byte(F8X16[c * 16 + i], OLED_DATA);
	}

	OLED_Set_Pos(x, y + 1);
	for (i = 0; i < 8; i++)
	{
		OLED_WR_Byte(F8X16[c * 16 + i + 8], OLED_DATA);
	}
}
// 显示字符串
// x,y:起点坐标
// size1:字体大小
//*chr:字符串起始地址
void OLED_ShowString(u8 x, u8 y, u8 *ch)
{
	u8 i = 0;
	while (ch[i] != '\0')
	{
		OLED_ShowChar(x, y, ch[i]);
		if (x > 128)
		{
			x = 0;
			y += 2;
		}
		x += 8;
		i++;
	}
}

// m^n
u32 OLED_Pow(u8 m, u8 n)
{
	u32 result = 1;
	while (n--)
	{
		result *= m;
	}
	return result;
}

// 显示数字
// x,y :起点坐标
// num :要显示的数字
// len :数字的位数
// size:字体大小
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size)
{
	u8 headflag = 0, temp, t;

	for (t = 0; t < len; t++)
	{
		temp = (num / OLED_Pow(10, len - t - 1)) % 10;
		if (headflag == 0 && t < (len - 1))
		{ // 排除掉num = 0 的时候
			if (temp == 0)
			{
				if (size == 16)
					OLED_ShowChar(x + (size / 2) * t, y, ' ');
				else if (size == 32)
					OLED_P32x64Str(x + (size / 2) * t, y, 11);
				continue;
			}
			else
			{
				headflag = 1;
			}
		}
		if (size == 16)
			OLED_ShowChar(x + (size / 2) * t, y, temp + '0');
		else if (size == 32)
			OLED_P32x64Str(x + (size / 2) * t, y, temp);
	}
}

void OLED_ShowChinese(u8 x, u8 y, u8 NO)
{
	unsigned char i = 0, addr = 0;
	OLED_Set_Pos(x, y);
	for (i = 0; i < 16; i++)
	{
		OLED_WR_Byte(Hzk[2 * NO][i], OLED_DATA);
		addr += 1;
	}
	OLED_Set_Pos(x, y + 1);
	for (i = 0; i < 16; i++)
	{
		OLED_WR_Byte(Hzk[2 * NO + 1][i], OLED_DATA);
		addr += 1;
	}
}
void OLED_P32x64Str(u8 x, u8 y, u8 dat)
{
	unsigned char i = 0;
	unsigned int adder = 64 * dat;

	if (x > 128)
	{
		x = 0;
		y++;
	}
	OLED_Set_Pos(x, y);
	for (i = 0; i < 16; i++)
	{
		OLED_WR_Byte(F16X32[adder], OLED_DATA);
		adder += 1;
	}

	OLED_Set_Pos(x, y + 1);
	for (i = 0; i < 16; i++)
	{
		OLED_WR_Byte(F16X32[adder], OLED_DATA);
		adder += 1;
	}

	OLED_Set_Pos(x, y + 2);
	for (i = 0; i < 16; i++)
	{
		OLED_WR_Byte(F16X32[adder], OLED_DATA);
		adder += 1;
	}

	OLED_Set_Pos(x, y + 3);
	for (i = 0; i < 16; i++)
	{
		OLED_WR_Byte(F16X32[adder], OLED_DATA);
		adder += 1;
	}
	// x+=1;
}
void OLED_wave(u8 Wave_sum) // 波形刷新
{
	static u8 i;
	OLED_WR_Byte(0xb6, OLED_CMD);					  // 设置页地址
	OLED_WR_Byte((i & 0x0f), OLED_CMD);				  // 设置显示位置—列低地址
	OLED_WR_Byte(((i & 0xf0) >> 4) | 0x10, OLED_CMD); // 设置显示位置—列高地址
	OLED_WR_Byte(PPG_wave_fig[Wave_sum * 2], OLED_DATA);

	OLED_WR_Byte(0xb7, OLED_CMD);							 // 设置页地址
	OLED_WR_Byte((i & 0x0f), OLED_CMD);						 // 设置显示位置—列低地址
	OLED_WR_Byte(((i & 0xf0) >> 4) | 0x10, OLED_CMD);		 // 设置显示位置—列高地址
	OLED_WR_Byte(PPG_wave_fig[Wave_sum * 2 + 1], OLED_DATA); // 显示数据,PPG_wave_fig[n]的值跟OLED寄存器配置的扫描方式相关
	i++;
	if (i > 127)
		i = 0;
}
// OLED的初始化 I2C
void OLED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 使能B端口时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 速度50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);			  // 初始化PB12,13C
	GPIO_SetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13);
	delay_ms(100);

	OLED_WR_Byte(0xAE, OLED_CMD); // 关闭显示
	OLED_WR_Byte(0x00, OLED_CMD); //---set low column address
	OLED_WR_Byte(0x10, OLED_CMD); //---set high column address
	OLED_WR_Byte(0xd5, OLED_CMD); // 设置时钟分频因子,震荡频率
	OLED_WR_Byte(0x80, OLED_CMD); //[3:0],分频因子;[7:4],震荡频率
	OLED_WR_Byte(0xA8, OLED_CMD); // 设置驱动路数(1 to 64)
	OLED_WR_Byte(0x3f, OLED_CMD); // 默认0X3F(1/64)
	OLED_WR_Byte(0xD3, OLED_CMD); // 设置显示偏移(0x00~0x3F)
	OLED_WR_Byte(0x00, OLED_CMD); // 不偏移

	OLED_WR_Byte(0x40, OLED_CMD); // 设置显示开始行 [5:0],行数.(0x00~0x3F)

	OLED_WR_Byte(0x8D, OLED_CMD); // 电荷泵设置
	OLED_WR_Byte(0x14, OLED_CMD); //--set(0x14) 开启/(0x10)关闭
	OLED_WR_Byte(0x20, OLED_CMD); // 设置内存地址模式
	OLED_WR_Byte(0x02, OLED_CMD); //[1:0],00，列地址模式;01，行地址模式;10,页地址模式
	OLED_WR_Byte(0xA1, OLED_CMD); // SEG映射       0xa0左右反置 0xa1正常
	OLED_WR_Byte(0xC8, OLED_CMD); // COM扫描方向   0xc0上下反置 0xc8正常
	OLED_WR_Byte(0xDA, OLED_CMD); // 设置COM硬件引脚配置
	OLED_WR_Byte(0x12, OLED_CMD); //[5:4]配置

	OLED_WR_Byte(0x81, OLED_CMD); // 对比度设置
	OLED_WR_Byte(0xEF, OLED_CMD); // 1~255;默认0X7F (亮度设置,越大越亮)
	OLED_WR_Byte(0xD9, OLED_CMD); // 设置预充电周期
	OLED_WR_Byte(0xF1, OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_WR_Byte(0xDB, OLED_CMD); // 设置VCOMH 电压倍率
	OLED_WR_Byte(0x30, OLED_CMD); // Set VCOM Deselect Level[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA6, OLED_CMD); //--set normal display
	OLED_Clear();
	OLED_WR_Byte(0xAF, OLED_CMD); // 开启显示
}
