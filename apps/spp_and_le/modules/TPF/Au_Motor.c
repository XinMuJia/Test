#include "system/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "update.h"
#include "TPH/Au_Motor.h"
#include "TPH/Au_Printf.h"
#include "TPH/Au_Timer.h"


uint8_t Motor_Pos = 0;

uint8_t Motor_Table[8][4] = {
    {1, 0, 0, 1},
    {0, 0, 0, 1},
    {0, 0, 1, 1},
    {0, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 1, 0, 0},
    {1, 1, 0, 0},
    {1, 0, 0, 0}
};

u8 step_sequence[8] = {0x05,0x01,0x09,0x08,0x0a,0x02,0x06,0x04}; 
// 定义全局变量：定时器句柄
static u16 motor_timer_id = 0;
static u32 motor_period_ms = 2; // 默认 2ms 步进周期

/*
  * @brief  向电机引脚写数据, 驱动电机转动
  * @param  pin:步进电机的四相(AM, BM, AP, BP)
  * @param  PinState:0 or 1
  * @return none	
  * @note   none
 */
void Motor_Write(int pin, int PinState)
{
    switch (pin) {
    case PIN_MOTOR_AM:
        gpio_direction_output(MOTOR_A_Pin, PinState);
        break;
    case PIN_MOTOR_BM:
        gpio_direction_output(MOTOR_B_Pin, PinState);
        break;
    case PIN_MOTOR_AP:
        gpio_direction_output(MOTOR_AN_Pin, PinState);
        break;
    case PIN_MOTOR_BP:
        gpio_direction_output(MOTOR_BN_Pin, PinState);
        break;
    default:
        break;
    }
}

/*
  * @brief  手动单步运行
  * @param  none
  * @return none	
  * @note   none
 */
void Motor_Run() 
{
    PRINT_DEBUG("Motor run!");
    
    Motor_Write(PIN_MOTOR_AM, Motor_Table[Motor_Pos][0]);
    Motor_Write(PIN_MOTOR_BM, Motor_Table[Motor_Pos][1]);
    Motor_Write(PIN_MOTOR_AP, Motor_Table[Motor_Pos][2]);
    Motor_Write(PIN_MOTOR_BP, Motor_Table[Motor_Pos][3]);
    
    //Debug_Print();
    

    Motor_Pos = (Motor_Pos + 1) & 0x07;
}

/*
  * @brief  手动多步运行
  * @param  steps:打印步数
  * @return none	
  * @note   none
 */
void Motor_Run_Steps(uint32_t steps)
{
    PRINT_DEBUG("Motor run steps:%d!", steps);
    while (steps--) {
        Motor_Write(PIN_MOTOR_AM, Motor_Table[Motor_Pos][0]);
        Motor_Write(PIN_MOTOR_BM, Motor_Table[Motor_Pos][1]);
        Motor_Write(PIN_MOTOR_AP, Motor_Table[Motor_Pos][2]);
        Motor_Write(PIN_MOTOR_BP, Motor_Table[Motor_Pos][3]); 
        //Debug_Print();
        
        Motor_Pos = (Motor_Pos + 1) & 0x07;
        
        /* 等待一步 */
        // us_delay(MOTOR_WAIT_TIME);
        us_delay(500);
        // os_time_dly(1);
    }
}

/*
  * @brief  电机初始化
  * @param  none
  * @return none	
  * @note   none
 */
void Init_Motor(void)
{
    PRINT_DEBUG("Init_Motor!");
    
    Motor_Write(PIN_MOTOR_AM, RESET);
    Motor_Write(PIN_MOTOR_BM, RESET);
    Motor_Write(PIN_MOTOR_AP, RESET);
    Motor_Write(PIN_MOTOR_BP, RESET); 
}

void Motor_Test(void)
{
    // MOTOR_STEP(0x00);
    PRINT_DEBUG("step_test!");
    for(int i = 0; i < 100; i++) {
        MOTOR_STEP(step_sequence[i % 8]);
        os_time_dly(3);
    } 
}