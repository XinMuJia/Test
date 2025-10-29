#ifndef     AU_MOTOR_H
#define     AU_MOTOR_H
#include "typedef.h"
#include "gpio.h"
#include "TPH/Au_Config.h"

#define MOTOR_B_Pin             IO_PORTA_01
#define MOTOR_BN_Pin            IO_PORTA_02
#define MOTOR_A_Pin             IO_PORTA_03
#define MOTOR_AN_Pin            IO_PORTA_04

// 全局变量声明
extern uint8_t Motor_Pos;
extern uint8_t Motor_Table[8][4];

// 函数声明
void Init_Motor(void);
void Motor_Start(void);
void Motor_Stop(void);
void Motor_Run(void);
void Motor_Run_Steps(uint32_t steps);

#endif      //AU_MOTOR_H