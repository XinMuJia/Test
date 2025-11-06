#ifndef __AU_PRINTF_H__
#define __AU_PRINTF_H__

#include "typedef.h"
#include "TPH/Au_Config.h"
#include "TPH/Au_TPG_Front.h"

//每行总点数
#define TPH_DOTS_PER_LINE 96
//每行字节长度
#define TPH_DI_LEN 8
//所有通道打印
#define ALL_STB_NUM 0xff
#define TPH_STB_NUM 0x01

//TPH数据时钟,DI,CLK
#define	GPIO_Port_TPH_DI			IO_PORTC_05
#define	GPIO_Port_TPH_CLK			IO_PORTC_04

//TPH锁存,LAT
#define	GPIO_Port_TPH_LAT			IO_PORTC_07
#define GPIO_Port_TPH_VH		    IO_PORTA_00


#define GPIO_Port_TPH_OC		    IO_PORTB_06
#define GPIO_Port_TPH_OC_2          IO_PORTB_08
#define GPIO_Port_EN_3V3            IO_PORTB_07
/*  OC使能,给光耦供电*/
#define OC_EN do { \
    gpio_direction_output(GPIO_Port_TPH_OC, 1); \
    gpio_direction_output(GPIO_Port_TPH_OC_2, 1); \
    gpio_direction_output(GPIO_Port_EN_3V3, 0); \
} while(0)
#define VCC_EN(temp)  gpio_direction_output(GPIO_Port_EN_3V3, (temp))

/*  STB使能,实际上只使用了通道1 */
#define STB1_Pin IO_PORTB_09
#define STB2_Pin IO_PORTD_01
#define STB3_Pin IO_PORTD_02
#define STB4_Pin IO_PORTD_03
#define STB5_Pin IO_PORTD_04
#define STB6_Pin IO_PORTD_05

#define ADC_Channel_Paper_Check   AD_CH_PB10 // 进纸检测接在PB10通道
#define ADC_Channel_Paper_Check2   AD_CH_PC6 

// 函数声明
void Set_Heat_Density(uint8_t Density);
void Init_Printer(void);
void TPH_PrintNum(u16 x);
void TPH_Loop1(void);
void Digital_Write(int pin, int pinState);
void TPH_PrintChar(u16 x,u8 o,u8 chr,u8 size);
void TPH_PrintString(u16 x,char *dp,u8 size);
void	TPH_Print1(u16 x);

#endif  //__AU_PRINTF_H__