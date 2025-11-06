/*******************************************************
***********2R打印机芯头测试型号*************************
***************软件版本2R_V2_0**************************
***JX_2R_01******JX_2R_01LL********JX_2R_08*************
***JX_2R_09******JX_2R_10**********JX_2R_12*************
***JX_2R_16******JX_2R_17**********JX_2R_22*************
@上电后轻触/校准键/选择所需要测试机芯型号
@选择需要的测试内容
		1.电机内阻：SpinO			
		2.光耦旁路电容测试:PaperC_EN		3.光耦旁路电容:PaperC	
		4.行打印总点数:PrintWidth				5.单次最大打印点数:OnePrint
		6.电机速度:Moto1Time						7.加热时间:HeatTime
		8.温敏电阻参数:Refer_TM					9.点电阻参数:Refer_Arr
		10.切刀使能:Tool_EN							11.VH电压选择:VH_High
@输出结构定义:UpStart_InitTypeDef;   
@串口测试:正确Correct  1
					错误Error		 0
					输出10*4Bit 
					char UpStartUSART[12];
*******************************************************/
#ifndef AU_START_H
#define AU_START_H

#include "Au_Config.h"

typedef struct
{
	u16	SpinO;
	FunctionalState	PaperC_EN;
	u8	PaperC;
	u16	PrintWidth;
	u16	OnePrint;
	u8	Moto1Time;
	u8	HeatTime;
	u16	Refer_TM;
	u16	Refer_Arr;
	FunctionalState	Tool_EN;
	FunctionalState	VH_High;
	FunctionalState RAY_High;
}UpStart_InitTypeDef;

//extern UpStart_InitTypeDef	UpStart_InitStructure;



#define TypeNum8		 			GPIO_ReadInputDataBit(GPIO_Port_Type,GPIO_Pin_Type_1)
#define TypeNum4		 			GPIO_ReadInputDataBit(GPIO_Port_Type,GPIO_Pin_Type_2)
#define TypeNum2		 			GPIO_ReadInputDataBit(GPIO_Port_Type,GPIO_Pin_Type_3)
#define TypeNum1		 			GPIO_ReadInputDataBit(GPIO_Port_Type,GPIO_Pin_Type_4)



void UpStart(void);

#endif // AU_START_H