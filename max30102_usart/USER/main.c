#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "max30102.h"
#include "myiic.h"
#include "algorithm.h"
#include "Oled_Dis.h"
#include <stdlib.h>

#define PPG_WAVE_COUNT 	4		// ÿ�ĸ���������һ����ʾ
#define PPG_MIN_COUNT 	0X3FFFF
u8 sata[2];
uint32_t aun_ir_buffer[500];  	// IR����,�����ԭʼ���ݻ�����
uint32_t aun_red_buffer[500]; 	// Red����,���ԭʼ���ݻ�����
int32_t n_ir_buffer_length;	  	// ���ݳ���

int32_t n_sp02;		  			// Ѫ��ֵ
int32_t n_heart_rate; 			// ����ֵ
int8_t ch_spo2_valid; 			// �㷨ִ�гɹ���־λ
int8_t ch_hr_valid;

int main(void)
{
	uint32_t un_min, un_max; 	// ԭʼ���ݵ����/��Сֵ
	uint32_t Wave_Range;	 	// ������ʾ���㱶��
	u16 i;
	u8 buf_flag = 0;			// �����Ƿ���Ч����ָ�뿪ʱ��0

	s8 Wave_sum; 				// OLED���δ�С
	u8 temp[6];	 			 	// ƴ�ֽ�
	u8 str[30];	 			 	// �ַ�����ʾ����
	u8 dis_hr = 60, dis_spo2 = 96;
	u8 wave;

	NVIC_Configuration(); 		// ��������ʼ������Ҫ���������ж����ȼ���
	delay_init();							// ��ʱ������ʼ��
	uart_init(115200);				// USART��ʼ����������115200
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

	n_ir_buffer_length = 500; // �������鳤��

	while (1)
	{
		i = 0; // ���ó�ʼֵ
		un_max = 0;
		un_min = PPG_MIN_COUNT;

		// ������ǰ��100->Ŀ���Ƕ���500�������е�ǰ��100������,������100���²ɼ���ֵ
		for (i = 100; i < 500; i++) 
		{
			aun_red_buffer[i - 100] = aun_red_buffer[i];
			aun_ir_buffer[i - 100] = aun_ir_buffer[i];

			if (i > 400)
			{
				if (un_min > aun_ir_buffer[i])
					un_min = aun_ir_buffer[i]; // �ҵ���Сֵ
				if (un_max < aun_ir_buffer[i])
					un_max = aun_ir_buffer[i]; // �ҵ����ֵ
			}
		}
		
    //number
		Wave_Range = un_max - un_min; // ���㲨�岨�ȵĲ�ֵ
		Wave_Range = Wave_Range / 14; // ���㱶��
		buf_flag = 0;

		for (i = 400; i < 500; i++) // ���100�������ݵ�����
		{
			wave++; // ����OLEDˢ���ٶ�
			while (MAX30102_INT == 1);

			max30102_FIFO_ReadBytes(REG_FIFO_DATA, temp);														 // aun_ir_buffer[i]
			aun_red_buffer[i] = (long)((long)((long)temp[0] & 0x03) << 16) | (long)temp[1] << 8 | (long)temp[2]; // �ɼ��ͺϲ��õ�ԭʼ����
			aun_ir_buffer[i] = (long)((long)((long)temp[3] & 0x03) << 16) | (long)temp[4] << 8 | (long)temp[5];

			// if (aun_red_buffer[i] > 5000) // ���˵��쳣ֵ
			if (aun_ir_buffer[i] > 0x15000 && aun_ir_buffer[i] < 0X30000 )     // ����0x15000С��0X30000ʱ,��Ϊ����������
			{
				if (aun_ir_buffer[i] > un_max) {
					un_max = (aun_ir_buffer[i]+0x100); 	// �����������ݵ����ֵ���ʵ��Ӵ�һ�����ֵ�����˵�С���ȵı仯��
				}					
				else if (aun_ir_buffer[i] < un_min)
				{
					un_min = (aun_ir_buffer[i]-0x100);	// �����������ݵ���Сֵ���ʵ�����һ����Сֵ�����˵�С���ȵı仯��
					Wave_Range = un_max - un_min;
					Wave_Range = Wave_Range / 14;
				}

				Wave_sum = (un_max - aun_ir_buffer[i]) / Wave_Range; // �õ�OLED��ʾ�Ĳ���ֵ
				buf_flag = 1;										 // ��־λ,ִ���㷨��,��ҪĿ���ǹ��˵���ָ�ſ�ʱ���쳣����
			}
			else
			{
				Wave_sum = 1;
				buf_flag = 0;			    
			}

			if (Wave_sum > 16)
				Wave_sum = 16; // ��ֵ����,����(��ֹ��״ͼԽ��-��״ͼΪ16�����)

			// printf("%d: %d, %d \r\n",i, aun_ir_buffer[i],aun_red_buffer[i]);//����������ε���λ����ʾ
			// printf("%d, %d, %d \r\n",un_max,un_min,Wave_Range);

			if (wave == PPG_WAVE_COUNT)			// ÿ��PPG_WAVE_COUNT�������ˢ��һ����ʾ
			{
				OLED_wave(Wave_sum); // ��ʾ����
				wave = 0;
			}
		}

		// ִ���㷨,ȥֱ��,�˲�,���㲨�η�ֵ��
		if (buf_flag) // ��ֹ����
		{
			maxim_heart_rate_and_oxygen_saturation(aun_ir_buffer, n_ir_buffer_length, aun_red_buffer, &n_sp02, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid);
		}
		if (buf_flag && n_sp02 < 100 && n_sp02 > 70) // ������ʾ����, ��ѧ������Ȼ��Ӱ��
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
