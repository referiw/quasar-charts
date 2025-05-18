#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "max30102.h"
#include "myiic.h"
#include "algorithm.h"
#include "Oled_Dis.h"
#include <stdlib.h>

#define PPG_WAVE_COUNT 	4		// 每四个采样进行一次显示
#define PPG_MIN_COUNT 	0X3FFFF
u8 sata[2];
uint32_t aun_ir_buffer[500];  	// IR数组,红外光原始数据缓存区
uint32_t aun_red_buffer[500]; 	// Red数组,红光原始数据缓存区
int32_t n_ir_buffer_length;	  	// 数据长度

int32_t n_sp02;		  			// 血氧值
int32_t n_heart_rate; 			// 心率值
int8_t ch_spo2_valid; 			// 算法执行成功标志位
int8_t ch_hr_valid;

int main(void)
{
	uint32_t un_min, un_max; 	// 原始数据的最大/最小值
	uint32_t Wave_Range;	 	// 用于显示计算倍率
	u16 i;
	u8 buf_flag = 0;			// 数据是否有效，手指离开时置0

	s8 Wave_sum; 				// OLED波形大小
	u8 temp[6];	 			 	// 拼字节
	u8 str[30];	 			 	// 字符串显示变量
	u8 dis_hr = 60, dis_spo2 = 96;
	u8 wave;

	NVIC_Configuration(); 		// 各函数初始化（主要是设置了中断优先级）
	delay_init();							// 延时函数初始化
	uart_init(115200);				// USART初始化，波特率115200
	OLED_Init();
	printf("start");
	max30102_init();

	OLED_ShowChinese(0, 0, 0);   //position
	OLED_ShowChinese(16, 0, 1);
	OLED_ShowChinese(64, 0, 2);
	OLED_ShowChinese(80, 0, 3);

    for(i = 0; i < 500; i++){
		aun_ir_buffer[i] = 0;
		aun_red_buffer[i] = 0;
	}

	n_ir_buffer_length = 500; // 更新数组长度

	while (1)
	{
		i = 0; // 重置初始值
		un_max = 0;
		un_min = PPG_MIN_COUNT;

		// 数组往前移100->目的是丢弃500个数组中的前面100个数组,并更新100个新采集的值
		for (i = 100; i < 500; i++) 
		{
			aun_red_buffer[i - 100] = aun_red_buffer[i];
			aun_ir_buffer[i - 100] = aun_ir_buffer[i];

			if (i > 400)
			{
				if (un_min > aun_ir_buffer[i])
					un_min = aun_ir_buffer[i]; // 找到最小值
				if (un_max < aun_ir_buffer[i])
					un_max = aun_ir_buffer[i]; // 找到最大值
			}
		}
		
    //number
		Wave_Range = un_max - un_min; // 计算波峰波谷的差值
		Wave_Range = Wave_Range / 14; // 计算倍率
		buf_flag = 0;

		for (i = 400; i < 500; i++) // 填充100个新数据到数组
		{
			wave++; // 减慢OLED刷新速度
			while (MAX30102_INT == 1);

			max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);														 // aun_ir_buffer[i]
			aun_red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 | (long)temp[2]; // 采集和合并得到原始数据
			aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 | (long)temp[5];

			// if (aun_red_buffer[i] > 5000) // 过滤掉异常值
			if (aun_ir_buffer[i] > 0x15000 && aun_ir_buffer[i] < 0X30000 )     // 大于0x15000小于0X30000时,认为是正常数据
			{
				if (aun_ir_buffer[i] > un_max) {
					un_max = (aun_ir_buffer[i]+0x100); 	// 保存这组数据的最大值（适当加大一点最大值，过滤掉小幅度的变化）
				}					
				else if (aun_ir_buffer[i] < un_min)
				{
					un_min = (aun_ir_buffer[i]-0x100);	// 保存这组数据的最小值（适当减少一点最小值，过滤掉小幅度的变化）
					Wave_Range = un_max - un_min;
					Wave_Range = Wave_Range / 14;
				}

				Wave_sum = (un_max - aun_ir_buffer[i]) / Wave_Range; // 得到OLED显示的波形值
				buf_flag = 1;										 // 标志位,执行算法用,主要目的是过滤掉手指放开时的异常数据
			}
			else
			{
				Wave_sum = 1;
				buf_flag = 0;			    
			}

			if (Wave_sum > 16)
				Wave_sum = 16; // 数值过大,纠正(防止柱状图越界-柱状图为16点阵高)

			// printf("%d: %d, %d \r\n",i, aun_ir_buffer[i],aun_red_buffer[i]);//输出脉搏波形到上位机显示
			// printf("%d, %d, %d \r\n",un_max,un_min,Wave_Range);

			if (wave == PPG_WAVE_COUNT)			// 每隔PPG_WAVE_COUNT个点阵后，刷新一下显示
			{
				OLED_wave(Wave_sum); // 显示波形
				wave = 0;
			}
		}

		// 执行算法,去直流,滤波,计算波形幅值等
		if (buf_flag) // 防止误判
		{
			maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
		}
		if (buf_flag && n_sp02 < 100 && n_sp02 > 70) // 划定显示区间, 光学会受自然光影响
		{
			dis_hr = n_heart_rate;  //jieguo
			dis_spo2 = n_sp02;
		}
		else
		{
			dis_hr = 00;
			dis_spo2 = 00;
		}

		printf("dis_hr = %d (%d), dis_spo2 = %d (%d)\r\n", dis_hr, ch_hr_valid, dis_spo2, ch_spo2_valid);

		if (dis_hr == 0 || dis_spo2 == 0)
		{
			//
			sprintf((char *)str, "                ");
			OLED_ShowString(0, 2, str);
			sprintf((char *)str, "----    ----    ");
			OLED_ShowString(0, 4, str);
			
			sata[0]=dis_hr;
			sata[1]=dis_spo2;
			USART_Send(USART1,sata,2);
		}
		else
		{
			OLED_ShowNum(0, 2, dis_hr, 3, 32);
			OLED_ShowNum(64, 2, dis_spo2, 2, 32 /* 16/32 */);
			
			OLED_ShowChar(106, 4, '%');
			sata[0]=dis_hr;
			sata[1]=dis_spo2;
			USART_Send(USART1,sata,2);
			
		}
	}
}
