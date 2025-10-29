#include "typedef.h"
#include "TPH/Au_Config.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "update.h"

bool State_Timeout = false;
bool Printer_Timeout = false;


struct Au_ID au_id;

#ifndef US_DELAY_LOOP_FACTOR
#define US_DELAY_LOOP_FACTOR    10u    // 经验值，越大忙等待越长，需校准
#endif
void us_delay_us(unsigned int us)
{
    if (!us) {
        return;
    }

    /* 如果大于等于 1000us，先用系统睡眠 ms 部分以节省 CPU */
    if (us >= 1000) {
        unsigned int ms = us / 1000;
        /* 注意：os_time_dly 接受 tick，不同平台 tick 长度不同。
           若有 ms 精度的睡眠函数，请用那个函数替换下面一行。 */
        os_time_dly(ms); // 按需替换为 ms 睡眠 API
        us = us % 1000;
    }

    /* 忙等待剩余的微秒（近似） */
    if (us) {
        volatile unsigned int i;
        unsigned int loops = us * US_DELAY_LOOP_FACTOR;
        for (i = 0; i < loops; i++) {
            (void)i;
        }
    }
}

/*
  * @brief  上报设备状态定时器回调
  * @param  arg
  * @return none	
  * @note   none
 */
void State_Timer_Callback(void const * arg)
{
    // PRINT_DEBUG("State_Timer CB now");
    State_Timeout = true;
}

/*
  * @brief  Timer初始化
  * @param  none
  * @return none	
  * @note   初始化并启动上报设备状态定时器,间歇触发,定时10s
 */
void Init_Timer(void)
{
    au_id.state_timer_id = sys_timer_add(NULL, State_Timer_Callback, 10000); // 10s
}

/*
  * @brief  返回上报状态标志
  * @param  none
  * @return none	
  * @note   none
 */
bool Get_State_Timeout(void)
{
    return State_Timeout;
}

/*
  * @brief  清除上报状态标志
  * @param  none
  * @return none	
  * @note   none
 */
void Clean_State_Timeout(void)
{
    State_Timeout = false;
}

/*
  * @brief  打印超时回调函数
  * @param  arg
  * @return none	
  * @note   none
 */
void Printer_Timer_Callback(void const * arg)
{
    // PRINT_DEBUG("Printer_Timer CB now");
    Printer_Timeout = true;
}

/*
  * @brief  启动打印超时定时器
  * @param  none
  * @return none	
  * @note   单次触发,定时20s
 */
void Open_Printer_Timeout_Timer(void)
{
    Printer_Timeout = false;
    au_id.printer_timer_id = sys_timer_add(NULL, Printer_Timer_Callback, PRINT_TIME); // 20s
}

/** @brief  停止打印超时定时器  * @param  none
  * @return none	
  * @note   none
 */
void Close_Printer_Timeout_Timer(void)
{
    // osTimerDelete(myStateTimerHandle);
    sys_timer_del(au_id.state_timer_id);
    
}

/*
  * @brief  获取打印超时标志
  * @param  none
  * @return none	
  * @note   none
 */
bool Get_Printer_Timeout_Status(void)
{
    return Printer_Timeout;
}
