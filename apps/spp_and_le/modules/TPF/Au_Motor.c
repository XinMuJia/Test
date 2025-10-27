#include "system/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "update.h"
#include "TPH/Au_Motor.h"

typedef enum {
    PIN_MOTOR_AM = 0,
    PIN_MOTOR_BM,
    PIN_MOTOR_AP,
    PIN_MOTOR_BP
} PIN_MOTOR_T;

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

// 定义全局变量：定时器句柄
static u16 motor_timer_id = 0;

/*
  * @brief  向电机引脚写数据, 驱动电机转动
  * @param  pin:步进电机的四相(AM, BM, AP, BP)
  * @param  PinState:0 or 1
  * @return none	
  * @note   none
 */
static void Motor_Write(int pin, int PinState)
{
    if (pin == PIN_MOTOR_AM) {
        // HAL_GPIO_WritePin(MOTOR_IN2_GPIO_Port, MOTOR_IN2_Pin, (GPIO_PinState)PinState);
        gpio_direction_output(MOTOR_A_Pin, PinState);
    }
    else if (pin == PIN_MOTOR_BM) {
        HAL_GPIO_WritePin(MOTOR_B_Pin, PinState);
    } 
   else if (pin == PIN_MOTOR_AP) {
        HAL_GPIO_WritePin(MOTOR_AN_Pin, PinState);
   } 
   else if (pin == PIN_MOTOR_BP) {
        HAL_GPIO_WritePin(MOTOR_BN_Pin, PinState);
   }
}

// 定时器回调函数
static void motor_timer_callback(void *arg)
{
    PRINT_DEBUG("Motor's timerHandler is running!");
    Motor_Write(PIN_MOTOR_AM, Motor_Table[Motor_Pos][0]);
    Motor_Write(PIN_MOTOR_BM, Motor_Table[Motor_Pos][1]);
    Motor_Write(PIN_MOTOR_AP, Motor_Table[Motor_Pos][2]);
    Motor_Write(PIN_MOTOR_BP, Motor_Table[Motor_Pos][3]);
    
    
    Motor_Pos++;
    if (Motor_Pos >= 8) {
        Motor_Pos = 0;
    }
}

/*
 * @brief  电机开始工作
 * @param  none
 * @return none	
 * @note   使用 sys_timer_add 创建周期性定时器，每 2ms 触发一次
 */
void Motor_Start(void)
{
    PRINT_DEBUG("Motor start");

    // 如果定时器已存在，则先删除旧的
    if (motor_timer_id) {
        sys_timer_del(motor_timer_id);
        motor_timer_id = 0;
    }

    // 创建新的周期性定时器，周期为 2ms
    motor_timer_id = sys_timer_add(NULL, motor_timer_callback, 2);  // 单位：毫秒
}


/*
  * @brief  电机停止工作
  * @param  none
  * @return none	
  * @note   none
 */
void Motor_Stop(void)
{
    PRINT_DEBUG("Motor stop!");
    
    Motor_Write(PIN_MOTOR_AM, RESET);
    Motor_Write(PIN_MOTOR_BM, RESET);
    Motor_Write(PIN_MOTOR_AP, RESET);
    Motor_Write(PIN_MOTOR_BP, RESET);
    
    if (motor_timer_id != NULL) { /* 停止定时器 */
        // osTimerStop(Motor_Timer);
        sys_timer_del(motor_timer_id);
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
    
    Motor_Pos++;
    if (Motor_Pos >= 8) {
        Motor_Pos = 0;
    }
}

/*
  * @brief  手动多步运行
  * @param  steps:打印步数
  * @return none	
  * @note   none
 */
void Motor_Run_Steps(uint32_t steps)
{
    PRINT_DEBUG("Motor run steps!");
    while (steps--) {
        Motor_Write(PIN_MOTOR_AM, Motor_Table[Motor_Pos][0]);
        Motor_Write(PIN_MOTOR_BM, Motor_Table[Motor_Pos][1]);
        Motor_Write(PIN_MOTOR_AP, Motor_Table[Motor_Pos][2]);
        Motor_Write(PIN_MOTOR_BP, Motor_Table[Motor_Pos][3]); 
        
        //Debug_Print();
        
        Motor_Pos++;
        if (Motor_Pos >= 8) {
            Motor_Pos = 0;
        }
        
        /* 等待一步 */
        us_delay(MOTOR_WAIT_TIME);
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
    
    //Debug_Print();
    //PRINT_DEBUG("Init_Motor finish!");
}

