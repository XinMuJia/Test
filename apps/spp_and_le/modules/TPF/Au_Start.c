/*******************************************************
***********3R打印机芯头测试型号*************************
***************软件版本3R_V4_0**************************
*@上电后轻触/校准键/选择所需要测试机芯型号
*@选择需要的测试内容
*		1.电机内阻：SpinO			
*		2.光耦旁路电容测试:PaperC_EN		3.光耦旁路电容:PaperC	
*		4.行打印总点数:PrintWidth				5.单次最大打印点数:OnePrint
*		6.电机速度:Moto1Time						7.加热时间:HeatTime
*		8.温敏电阻参数:Refer_TM					9.点电阻参数:Refer_Arr
*		10.切刀使能:Tool_EN							11.VH电压选择:VH_High
*@0.0625mm*(1000/(Moto1Time*0.1))=走纸速度
*@输出结构定义:UpStart_InitTypeDef;   
*@串口测试:正确Correct  		01-22
*					错误Error		 		0xAA
*					未收录Failure		0x55
*					输出10*4Bit 
*					char UpStartUSART[12];
*					
*******************************************************/
#include "TPH/Au_Start.h"

u8 UpStart_Init(void);
void UpUSART_Num(void);

u16 	UpStartUSART[13];										//启动数据缓存，可用串口输出
u16 	UpStart_Num;												//Up型号选择序列
char	*Type_ID;
BitAction Link_G_en;
UpStart_InitTypeDef UpStart_InitStructure;
BitAction	STB_OFF;		
BitAction	STB_ON;		
/************************************
* 函数：u8 UpStart(void)
* 功能：测试机芯型号选择参数设置
* 入口参数：无
* 出口参数：正确Correct  		01-22
*						错误Error		 		0xAA
*						未收录Failure		0x55
*************************************/
void UpStart(void)
{
	UpStart_Num=UpStart_Init();
	UpUSART_Num();
}

/************************************
* 函数：u8 UpStart_Init(u8 Up_Num)
* 功能：测试机芯参数设置
* 入口参数：Up型号选择序列，Up_Num
* 出口参数：正确Correct  		01-22
*						错误Error		 		0xAA
*						未收录Failure		0x55
*************************************/
u8 UpStart_Init(void)
{
			UpStart_InitStructure.HeatTime	=	8;					//加热时间（HeatTime+1)*0.1ms          
			UpStart_InitStructure.Moto1Time	=	5;					//初始电机步进间隔，单位：*0.1ms  //速度公式：(1/(Moto1Time*0.1))*0.03125*1000= mm/s     
			UpStart_InitStructure.OnePrint	=	96;					//最多一次加热点数参数64，测试96
			UpStart_InitStructure.PaperC	=	100;				//光耦旁路电容值100pF，单位：pF
			UpStart_InitStructure.PaperC_EN	=	FUN_DISABLE;		//不使能旁路电容检测
			UpStart_InitStructure.PrintWidth=	384;				//一行总加热点数
			UpStart_InitStructure.Refer_Arr	=	176;				//点电阻值，单位：R，欧姆
			UpStart_InitStructure.Refer_TM	=	320;				//热敏电阻参数30KR，单位：R，欧姆
			UpStart_InitStructure.SpinO		=	3500;				//电机电感参数，单位：uH
			UpStart_InitStructure.Tool_EN	=	FUN_DISABLE;		//不使能切刀
			UpStart_InitStructure.VH_High	=	FUN_DISABLE;		//使能7200mV高压电源
			UpStart_InitStructure.RAY_High	=	FUN_DISABLE;		//高电位使能
			STB_OFF	=		Bit_RESET;								//STB扇区高开低关
			STB_ON	=		Bit_SET;
			Link_G_en	=	Bit_SET;								//接地测试
			Type_ID	=	"AG03";
			return	0x01;
}



/************************************
* 函数：void UpUSART_Num(void)
* 功能：机芯参数设置输入缓存
* 入口参数：无
* 出口参数：外部调用 UpStartUSART[]
*************************************/
void UpUSART_Num(void)
{
	//UpStart_InitTypeDef UpStart_InitStructure;
	UpStartUSART[0]	=	UpStart_Num;											//调用机芯序列
	UpStartUSART[1]	=	UpStart_InitStructure.HeatTime;
	UpStartUSART[2]	=	UpStart_InitStructure.Moto1Time;	
	UpStartUSART[3]	=	UpStart_InitStructure.OnePrint;	
	UpStartUSART[4]	=	UpStart_InitStructure.PaperC;		
	UpStartUSART[5]	=	UpStart_InitStructure.PaperC_EN;	
	UpStartUSART[6]	=	UpStart_InitStructure.PrintWidth;
	UpStartUSART[7]	=	UpStart_InitStructure.Refer_Arr;	
	UpStartUSART[8]	=	UpStart_InitStructure.Refer_TM;	
	UpStartUSART[9]	=	UpStart_InitStructure.SpinO;			
	UpStartUSART[10]=	UpStart_InitStructure.Tool_EN;	
	UpStartUSART[11]=	UpStart_InitStructure.VH_High;	
	UpStartUSART[12]=	UpStart_InitStructure.RAY_High;
}







