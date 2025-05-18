#include "Oled_Dis.h"
#include "stdlib.h"
#include "oled_font.h"
#include "delay.h"

// ��ʱ
void IIC_delay(void)
{
	u8 t = 10;
	while (t--)
		;
}
// ��ʼ�ź�
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
// �����ź�
void I2C_Stop(void)
{
	OLED_SDA_Clr();
	OLED_SCL_Set();
	IIC_delay();
	OLED_SDA_Set();
}
// �ȴ��ź���Ӧ
void I2C_WaitAck(void) // �������źŵĵ�ƽ
{
	OLED_SDA_Set();
	IIC_delay();
	OLED_SCL_Set();
	IIC_delay();
	OLED_SCL_Clr();
	IIC_delay();
}
// д��һ���ֽ�
void Send_Byte(u8 dat)
{
	u8 i;
	for (i = 0; i < 8; i++)
	{
		if (dat & 0x80)
		{ // ��dat��8λ�����λ����д��
			OLED_SDA_Set();
		}
		else
		{
			OLED_SDA_Clr();
		}
		IIC_delay();
		OLED_SCL_Set();
		IIC_delay();
		OLED_SCL_Clr(); // ��ʱ���ź�����Ϊ�͵�ƽ
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

// ���Ժ���
void OLED_ColorTurn(u8 i)
{
	if (i == 0)
	{
		OLED_WR_Byte(0xA6, OLED_CMD);
	} // ������ʾ
	if (i == 1)
	{
		OLED_WR_Byte(0xA7, OLED_CMD);
	} // ��ɫ��ʾ
}
// ��Ļ��ת180��
void OLED_DisplayTurn(u8 i)
{
	if (i == 0)
	{
		OLED_WR_Byte(0xC8, OLED_CMD); // ������ʾ
		OLED_WR_Byte(0xA1, OLED_CMD);
	}
	if (i == 1)
	{
		OLED_WR_Byte(0xC0, OLED_CMD); // ��ת��ʾ
		OLED_WR_Byte(0xA0, OLED_CMD);
	}
}
// ����һ���ֽ�
// mode:����/�����־ 0,��ʾ����;1,��ʾ����;
// void OLED_WR_Byte(u8 dat,u8 mode){}
void OLED_Set_Pos(u8 x, u8 y)
{
	OLED_WR_Byte(0xb0 + y, OLED_CMD);
	OLED_WR_Byte((x & 0x0f), OLED_CMD);
	OLED_WR_Byte((x & 0xf0) >> 4 | 0x10, OLED_CMD);
}
// ����OLED��ʾ
void OLED_ON(void)
{
	OLED_WR_Byte(0x8D, OLED_CMD); // ��ɱ�ʹ��
	OLED_WR_Byte(0x14, OLED_CMD); // ������ɱ�
	OLED_WR_Byte(0xAF, OLED_CMD); // ������Ļ
}
// �ر�OLED��ʾ
void OLED_OFF(void)
{
	OLED_WR_Byte(0x8D, OLED_CMD); // ��ɱ�ʹ��
	OLED_WR_Byte(0x10, OLED_CMD); // �رյ�ɱ�
	OLED_WR_Byte(0xAE, OLED_CMD); // �ر���Ļ
}
// ��������
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
// ��ָ��λ����ʾһ���ַ�,���������ַ�
// x:0~127
// y:0~63
// size1:ѡ������ 6x8/6x12/8x16/12x24
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
// ��ʾ�ַ���
// x,y:�������
// size1:�����С
//*chr:�ַ�����ʼ��ַ
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

// ��ʾ����
// x,y :�������
// num :Ҫ��ʾ������
// len :���ֵ�λ��
// size:�����С
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size)
{
	u8 headflag = 0, temp, t;

	for (t = 0; t < len; t++)
	{
		temp = (num / OLED_Pow(10, len - t - 1)) % 10;
		if (headflag == 0 && t < (len - 1))
		{ // �ų���num = 0 ��ʱ��
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
void OLED_wave(u8 Wave_sum) // ����ˢ��
{
	static u8 i;
	OLED_WR_Byte(0xb6, OLED_CMD);					  // ����ҳ��ַ
	OLED_WR_Byte((i & 0x0f), OLED_CMD);				  // ������ʾλ�á��е͵�ַ
	OLED_WR_Byte(((i & 0xf0) >> 4) | 0x10, OLED_CMD); // ������ʾλ�á��иߵ�ַ
	OLED_WR_Byte(PPG_wave_fig[Wave_sum * 2], OLED_DATA);

	OLED_WR_Byte(0xb7, OLED_CMD);							 // ����ҳ��ַ
	OLED_WR_Byte((i & 0x0f), OLED_CMD);						 // ������ʾλ�á��е͵�ַ
	OLED_WR_Byte(((i & 0xf0) >> 4) | 0x10, OLED_CMD);		 // ������ʾλ�á��иߵ�ַ
	OLED_WR_Byte(PPG_wave_fig[Wave_sum * 2 + 1], OLED_DATA); // ��ʾ����,PPG_wave_fig[n]��ֵ��OLED�Ĵ������õ�ɨ�跽ʽ���
	i++;
	if (i > 127)
		i = 0;
}
// OLED�ĳ�ʼ�� I2C
void OLED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // ʹ��B�˿�ʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;  // �������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // �ٶ�50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);			  // ��ʼ��PB12,13C
	GPIO_SetBits(GPIOB, GPIO_Pin_12 | GPIO_Pin_13);
	delay_ms(100);

	OLED_WR_Byte(0xAE, OLED_CMD); // �ر���ʾ
	OLED_WR_Byte(0x00, OLED_CMD); //---set low column address
	OLED_WR_Byte(0x10, OLED_CMD); //---set high column address
	OLED_WR_Byte(0xd5, OLED_CMD); // ����ʱ�ӷ�Ƶ����,��Ƶ��
	OLED_WR_Byte(0x80, OLED_CMD); //[3:0],��Ƶ����;[7:4],��Ƶ��
	OLED_WR_Byte(0xA8, OLED_CMD); // ��������·��(1 to 64)
	OLED_WR_Byte(0x3f, OLED_CMD); // Ĭ��0X3F(1/64)
	OLED_WR_Byte(0xD3, OLED_CMD); // ������ʾƫ��(0x00~0x3F)
	OLED_WR_Byte(0x00, OLED_CMD); // ��ƫ��

	OLED_WR_Byte(0x40, OLED_CMD); // ������ʾ��ʼ�� [5:0],����.(0x00~0x3F)

	OLED_WR_Byte(0x8D, OLED_CMD); // ��ɱ�����
	OLED_WR_Byte(0x14, OLED_CMD); //--set(0x14) ����/(0x10)�ر�
	OLED_WR_Byte(0x20, OLED_CMD); // �����ڴ��ַģʽ
	OLED_WR_Byte(0x02, OLED_CMD); //[1:0],00���е�ַģʽ;01���е�ַģʽ;10,ҳ��ַģʽ
	OLED_WR_Byte(0xA1, OLED_CMD); // SEGӳ��       0xa0���ҷ��� 0xa1����
	OLED_WR_Byte(0xC8, OLED_CMD); // COMɨ�跽��   0xc0���·��� 0xc8����
	OLED_WR_Byte(0xDA, OLED_CMD); // ����COMӲ����������
	OLED_WR_Byte(0x12, OLED_CMD); //[5:4]����

	OLED_WR_Byte(0x81, OLED_CMD); // �Աȶ�����
	OLED_WR_Byte(0xEF, OLED_CMD); // 1~255;Ĭ��0X7F (��������,Խ��Խ��)
	OLED_WR_Byte(0xD9, OLED_CMD); // ����Ԥ�������
	OLED_WR_Byte(0xF1, OLED_CMD); //[3:0],PHASE 1;[7:4],PHASE 2;
	OLED_WR_Byte(0xDB, OLED_CMD); // ����VCOMH ��ѹ����
	OLED_WR_Byte(0x30, OLED_CMD); // Set VCOM Deselect Level[6:4] 000,0.65*vcc;001,0.77*vcc;011,0.83*vcc;

	OLED_WR_Byte(0xA6, OLED_CMD); //--set normal display
	OLED_Clear();
	OLED_WR_Byte(0xAF, OLED_CMD); // ������ʾ
}
