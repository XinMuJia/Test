#include "system/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "update.h"
#include "TPH/Au_Printf.h"
#include "TPH/Au_Motor.h"
#include "TPH/Au_Timer.h"
#include "TPH/Au_Spi.h"

#define LOW               0x0
#define HIGH              0x1

float addTime[6] = {0};
//点数-增加时间系数
#define kAddTime 0.001

//根据打印头实际打印效果修改打印时间偏移值
#define STB1_ADDTIME 0
#define STB2_ADDTIME 0
#define STB3_ADDTIME 0
#define STB4_ADDTIME 0
#define STB5_ADDTIME 0
#define STB6_ADDTIME 0

// 日志系统定义
#define LOG_TAG_CONST       APP
#define LOG_TAG             "[AU_PRINTF]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

/*  函数声明*/
extern	u16	                It_Num;
extern	u16	                UpStartUSART[14];
extern	FunctionalState	    TPH_EN;
extern	FunctionalState	    MOTO_EN;
extern	BitAction	        STB_ON,STB_OFF;
extern 	FunctionalState     MOTO_EN_It;
extern 	FunctionalState     TPH_EN_It;
extern 	FunctionalState 	MOTO_2Esc;

//热密度
uint8_t Heat_Density = 64;

//步进动作
u8	res;

//打印字符
extern char TPH_PrintChar[10][24];

/*
  * @brief  引脚写入电平
  * @param  pin:引脚, pinState:引脚状态
  * @return none	
  * @note   none
 */
void Digital_Write(int pin, int pinState)
{
    switch (pin) {
        case PIN_STB1:
            gpio_direction_output(STB1_Pin, pinState);
            break;
        case PIN_STB2:
            gpio_direction_output(STB2_Pin, pinState);
            break;
        case PIN_STB3:
            gpio_direction_output(STB3_Pin, pinState);
            break;
        case PIN_STB4:
            gpio_direction_output(PIN_STB4, pinState);
            break;
        case PIN_STB5:
            gpio_direction_output(PIN_STB5, pinState);
            break;
        case PIN_STB6:
            gpio_direction_output(PIN_STB6, pinState);
            break;
        case PIN_LAT:
            gpio_direction_output(GPIO_Port_TPH_LAT, pinState);
            break;
        default:
            break;
    }
}

/*
  * @brief  VHEN引脚写入电平
  * @param  pin, pinState
  * @return none	
  * @note   none
 */
static void Digital_Write_Vhen(int pin,int PinState){
    PRINT_DEBUG("START VHEN");
	gpio_direction_output(GPIO_Port_TPH_VH,PinState);
}

/*
  * @brief  失能所有通道
  * @param  none
  * @return none	
  * @note   none
 */
static void Set_Stb_Unable(void)
{
    Digital_Write(PIN_STB1, HIGH); // 高电平失能
    Digital_Write(PIN_STB2, LOW);
    Digital_Write(PIN_STB3, LOW);
    Digital_Write(PIN_STB4, LOW);
    Digital_Write(PIN_STB5, LOW);
    Digital_Write(PIN_STB6, LOW);
}

/*
  * @brief  设置打印热密度
  * @param  Density:热密度
  * @return none	
  * @note   none
 */
void Set_Heat_Density(uint8_t Density)
{
    PRINT_DEBUG("打印密度设置%d\n", Density);
    Heat_Density = Density;
}

//打印黑块
void TPH_Loop1(void)
{
    u8 DATA1;
	u8 data1;
	u8	k,m;
	TPH_EN	=	FUN_ENABLE;
	MOTO_EN	=	FUN_ENABLE;
	do
	{
	for(m=8;m>0;m--)
		{
			data1=0XFF;
            // Spi_Command(data1);
            for(k=0;k<8;k++)																//输入一个字节数据
			{
				DATA1=0x00;
				if(data1	&	0x80)															//高位在前
				{
					DATA1=0x88;
				}
				TPH_WR_Byte(DATA1);
				data1 <<= 1;
			}
		}
		// TPH_Space(256);
        TPH_Space(32);
		TPH_HLAT(Bit_RESET);																	//数据锁存
		us_delay(LAT_TIME);
		TPH_HLAT(Bit_SET); 
	}while(MOTO_EN_It	==	FUN_ENABLE && MOTO_EN	==	FUN_ENABLE);

		TPH_PrintNum(0);
}

/*******************************************************************************
    * 函数说明：TPH空行打印
    * 入口数据：x 	空打印点数
    * 返回值：  无
*******************************************************************************/
void TPH_Space(u16	x)
{
    u16	c;
    for(c=x;c>0;c--)
    {
        TPH_WR_Byte(0x00);
    }
}



/*
  * @brief  打印模块初始化
  * @param  none
  * @return none	
  * @note   none
 */
void Init_Printer(void)
{
    Init_Motor(); // 初始化电机
    Set_Stb_Unable(); // 失能所有通道
    Digital_Write(PIN_LAT, HIGH); // 锁存引脚高电平
    gpio_direction_output(GPIO_Port_TPH_DI, LOW); // 数据引脚低电平
    gpio_direction_output(GPIO_Port_TPH_CLK, LOW); // 时钟引脚低电平
    Digital_Write_Vhen(PIN_VHEN, HIGH);
    OC_EN;
    TPH_EN	=	FUN_ENABLE;
	MOTO_EN	=	FUN_ENABLE;	
	res	=	0;
    os_time_dly(50);
}

void TPH_Esc(void)
{
	Digital_Write_Vhen(PIN_VHEN, LOW);
	TPH_EN	=	FUN_DISABLE;
	MOTO_EN	=	FUN_DISABLE;
	// MOTOR_STEP(0x06);	
    // 电机归零/释放
    MOTOR_STEP(0x00);  // 所有相位断电
    os_time_dly(10);
}


/******************************************************************************
*	函数说明：TPH打印开始
*	入口数据：x		已输入行点阵数
*	返回值：  无
*	说明：
			补足行点数
			锁存
			加热
			走步
******************************************************************************/
extern	BitAction	PAPER_Key;
void	TPH_PrintNum(u16 x)
{
	
		TPH_EN_It	= 	FUN_ENABLE;
		MOTO_EN_It = 	FUN_ENABLE;														//关闭中断执行中MOTO的使能
/*	TPH_EN_It	= 	ENABLE;
	MOTO_EN_It = 	ENABLE;														//关闭中断执行中MOTO的使能
	while(MOTO_EN_It == ENABLE)
	{
		if(PAPER_Key	==	Bit_RESET)
		{
			break;
		}
	}*/
}