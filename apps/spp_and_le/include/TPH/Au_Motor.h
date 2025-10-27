#ifndef     AU_MOTOR_H
#define     AU_MOTOR_H
#include "typedef.h"
#include "gpio.h"

#define MOTOR_B_Pin             IO_PORTA_01
#define MOTOR_BN_Pin            IO_PORTA_02
#define MOTOR_A_Pin             IO_PORTA_03
#define MOTOR_AN_Pin            IO_PORTA_04

#define RESET                0
#define PRINT_DEBUG(...)    printf(__VA_ARGS__);

//打印头电源升压控制引脚
#define PIN_VHEN 17
#define PRINT_TIME 2600         //打印加热时间
#define PRINT_END_TIME 200      //冷却时间
#define MOTOR_WAIT_TIME 4000    //电机一步时间
#define LAT_TIME 1              //数据锁存时间

#endif      //AU_MOTOR_H