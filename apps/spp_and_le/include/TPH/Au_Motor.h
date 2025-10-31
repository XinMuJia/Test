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
void Motor_Test(void);
void Motor_SmoothSetSpeed(u32 target_period_ms, u32 step_delay_ms);

// 电机步进序列
// #define MOTOR_STEP_SEQUENCE {0x05,0x01,0x09,0x08,0x0a,0x02,0x06,0x04};

// 电机单步执行宏定义
// #define MOTOR_STEP(temp) ( \
//     gpio_write(PIN_MOTOR_AM, (BitAction)(temp & 0x01)), \
//     gpio_write(PIN_MOTOR_AP, (BitAction)((temp & 0x02) >> 1)), \
//     gpio_write(PIN_MOTOR_BM,(BitAction)((temp & 0x04) >> 2)), \
//     gpio_write(PIN_MOTOR_BP, (BitAction)((temp & 0x08) >> 3)) \
// )

#define MOTOR_STEP(temp) do { \
    Motor_Write(PIN_MOTOR_AM, (BitAction)(temp & 0x01)); \
    Motor_Write(PIN_MOTOR_AP, (BitAction)((temp & 0x02) >> 1)); \
    Motor_Write(PIN_MOTOR_BM, (BitAction)((temp & 0x04) >> 2)); \
    Motor_Write(PIN_MOTOR_BP, (BitAction)((temp & 0x08) >> 3)); \
} while(0)

#endif      //AU_MOTOR_H