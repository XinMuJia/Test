#ifndef __AU_PRINTF_H__
#define __AU_PRINTF_H__

#include "typedef.h"
#include "TPH/Au_Config.h"

//每行总点数
#define TPH_DOTS_PER_LINE 96
//每行字节长度
#define TPH_DI_LEN 20
//所有通道打印
#define ALL_STB_NUM 0xff
#define TPH_STB_NUM 0x01

//TPH数据时钟,DI,CLK
#define	GPIO_Port_TPH_DI			IO_PORTC_05
#define	GPIO_Port_TPH_CLK			IO_PORTC_04

//TPH锁存,LAT
#define	GPIO_Port_TPH_LAT			IO_PORTC_07

#define GPIO_Port_TPH_VH		    IO_PORTA_00
#define VH_EN(i) gpio_direction_output(GPIO_Port_TPH_VH, i)

#define GPIO_Port_TPH_OC		    IO_PORTB_06
#define GPIO_Port_TPH_OC_2          IO_PORTB_07
#define OC_EN do { \
    gpio_direction_output(GPIO_Port_TPH_OC, 1); \
    gpio_direction_output(GPIO_Port_TPH_OC_2, 1); \
} while(0)

#define STB1_Pin IO_PORTB_08
#define STB2_Pin IO_PORTD_01
#define STB3_Pin IO_PORTD_02
#define STB4_Pin IO_PORTD_03
#define STB5_Pin IO_PORTD_04
#define STB6_Pin IO_PORTD_05

#define ADC_Channel_Paper_Check   IO_PORTB_11 // 进纸检测接在PB11通道

// 函数声明
void Set_Heat_Density(uint8_t Density);
bool Move_And_Start_STB(bool need_stop, uint8_t STBnum);
void Start_Printing_By_QueueBuffer(void);
void Start_Printing_By_OneSTB(uint8_t STBnum, uint8_t* Data, uint32_t Len);
void TestSTB(void);
void Init_Printer(void);

#endif  //__AU_PRINTF_H__