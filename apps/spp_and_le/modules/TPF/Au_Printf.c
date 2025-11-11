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

//宽度
// #define TPH_WRITE UpStartUSART[6]

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
u16	kos;

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

/******************************************************************************
*	函数说明：检查行打印字符是否溢出
*	入口数据：x  当前已需打印点行数
*	          b  行尾预留白
*	返回值：  ENABLE		有预留
*						DISABLE		已满，另起一行
******************************************************************************/
FunctionalState TPH_Column(u16 x,u16 b)
{
	if((x+b)	<	UpStartUSART[6])
	{
		return	FUN_ENABLE;
	}
	else
	{
		return	FUN_DISABLE;
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
    Digital_Write(PIN_STB1, LOW); // 高电平失能
    // Digital_Write(PIN_STB2, LOW);
    // Digital_Write(PIN_STB3, LOW);
    // Digital_Write(PIN_STB4, LOW);
    // Digital_Write(PIN_STB5, LOW);
    // Digital_Write(PIN_STB6, LOW);
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
	for(m=4;m>0;m--)
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
        TPH_Space(64);
		TPH_HLAT(Bit_RESET);																	//数据锁存
		us_delay(LAT_TIME);
		TPH_HLAT(Bit_SET); 
	}while(MOTO_EN_It	==	FUN_ENABLE && MOTO_EN	==	FUN_ENABLE);

		TPH_PrintNum(0);
}

//打印黑块
void TPH_Loop2(void)
{
    u8 DATA1;
	u8 data1;
	u8	k,m;
	TPH_EN	=	FUN_ENABLE;
	MOTO_EN	=	FUN_ENABLE;
	do
	{
	TPH_Space(32);
	for(m=4;m>0;m--)
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
		TPH_Space(32);
		TPH_HLAT(Bit_RESET);																	//数据锁存
		us_delay(LAT_TIME);
		TPH_HLAT(Bit_SET); 
	}while(MOTO_EN_It	==	FUN_ENABLE && MOTO_EN	==	FUN_ENABLE);

		TPH_PrintNum(0);
}

/******************************************************************************
*	函数说明：TPH打印字符函数
*	入口数据：x		 起始坐标
*						o		 打印高度
*           chr  写入的字符
*           size 字符的大小
* 返回值：  无
******************************************************************************/
void TPH_PrintChar(u16 x,u8 o,u8 chr,u8 size)
{
	u8 c,j,k;
	u8 data1,DATA1;
	c=chr-' ';																				//得到偏移后的值
	o*=size/16;																				//计算当前打印的行数
	for(j=0;j < size/16;j++)													            //当前字符打印对需要的字节数
	{	
		if(size==16)data1=TPH_F8X16[c][o];							                        //查找字库
		else if(size==32)data1=TPH_F12X24[c][o+j];
		else return;																		//未找到相应字符大小直接返回
	//	j==1?(k=4):(k=0);
		us_delay(20);
		for(k=0;k<8;k++)																    //输入一个字节数据
		{
			DATA1=0x00;
			if(data1	&	0x80)															//高位在前
			{
				kos++;
				DATA1=0x88;
			}
			TPH_WR_Byte(DATA1);
			data1 <<= 1;
		}
	}
}

extern 	u8 TPH_LAN_TEST;
/******************************************************************************
*	函数说明：TPH打印字符串
*	入口数据：x  		起始坐标
            *dp 	写入的字符串
            size	字符的大小
*	返回值：  无
******************************************************************************/
void 	TPH_PrintString(u16 x,char *dp,u8 size)
{
	u8	o,k,Ta;
	u8	Wide,Tall,Column;
	u16	z;
	u16	Header;
	FunctionalState	loop;
	kos=0;
	Ta=size;
	Ta>16?(Ta=32):(Ta=16);
	Wide=16;																			//ASCII 字符宽														
	Tall=size;																			//ASCII 字符高
	Header=x;																			//前留白
	Column=0;																			//字符串所打印行数
	TPH_EN	=	FUN_ENABLE;
	MOTO_EN	=	FUN_ENABLE;	
	do
	{
		loop=	FUN_DISABLE;															//字符串打印循环初始化
		for(o=0;o<Tall;o++)																//打印字符高度循环
		{
			k	=	Column;																//现在打印的字符寻址位
			z	=	Header;																//行打印点数初始							
			TPH_Space(Header);
			while(dp[k]!='\0')												
			{
				// if(TPH_Column(z,Wide) !=	FUN_ENABLE)						
				// {
				// 	loop	=	FUN_ENABLE;												//继续循环
				// 	break;
				// }
				TPH_PrintChar(Header,o,dp[k],Ta);	
				k++;																	//字符寻址位计数
				z+=Wide;																//已打印行点阵数计数
				if(kos>=96)																//当打印有效位等于96点 或者 数据传输完毕时 进行一次加热
				{
						TPH_Print1(96-z);
						TPH_Space(z);
						kos=0;
				}
				if(TPH_Column(z,0) !=	FUN_ENABLE)						
				{
					loop	=	FUN_ENABLE;												//继续循环
					break;
				}
			}
				if(TPH_LAN_TEST	==	1)
				{
					TPH_Space(96-z);		
					TPH_HLAT(Bit_SET); 													//LAN测试
				}
				else
				{
					TPH_Space(96-z);		
					TPH_HLAT(Bit_RESET);																//数据锁存
					us_delay(LAT_TIME);
					TPH_HLAT(Bit_SET); 
				}
			while(MOTO_EN_It	==	FUN_ENABLE && MOTO_EN == FUN_ENABLE);
				
			TPH_PrintNum(0);
			kos=0;
		}
		Column	=	k;																//寻址位缓存
	}while(loop	==	FUN_ENABLE && dp[k]!='\0');				//字符打印完毕，结束循环
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


/*******************************************************************************
    * 函数说明：TPH开始打印
    * 入口数据：x 	打印点数
    * 返回值：  无
*******************************************************************************/
void	TPH_Print1(u16 x)
{
	TPH_Space(x);																	//补足剩下所对齐的点阵数
	while(TPH_EN_It != DISABLE);
	TPH_HLAT(Bit_RESET);																	//数据锁存
    us_delay(LAT_TIME);
	TPH_HLAT(Bit_SET); 
	It_Num=0;
	TPH_EN_It	= ENABLE;
}


/*
  * @brief  打印模块初始化
  * @param  none
  * @return none	
  * @note   none
 */
void Init_Printer(void)
{
    Set_Stb_Unable(); // 失能所有通道
    Digital_Write(PIN_LAT, HIGH); // 锁存引脚高电平
    gpio_direction_output(GPIO_Port_TPH_DI, LOW); // 数据引脚低电平
    gpio_direction_output(GPIO_Port_TPH_CLK, LOW); // 时钟引脚低电平
    Digital_Write_Vhen(PIN_VHEN, HIGH);
    OC_EN;
    TPH_EN	=	FUN_ENABLE;
	MOTO_EN	=	FUN_ENABLE;	
	res	=	0;
}

void TPH_Esc(void)
{
	Digital_Write_Vhen(PIN_VHEN, LOW);
	TPH_EN	=	FUN_DISABLE;
	MOTO_EN	=	FUN_DISABLE;
	// MOTOR_STEP(0x06);	
    // 电机归零/释放
    MOTOR_STEP(0x00);  // 所有相位断电
}


void TPH_Start(void)
{
		TPH_EN_It	= 	FUN_ENABLE;
		MOTO_EN_It 	= 	FUN_ENABLE;
		Digital_Write_Vhen(PIN_VHEN, HIGH);
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