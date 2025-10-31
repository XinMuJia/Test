#include "typedef.h"
#include "TPH/Au_Config.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "update.h"
#include "TPH/Au_Timer.h"
#include "TPH/Au_Printf.h"

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

    /* 对于较短延时也考虑使用系统调度 */
    if (us >= 1000) {
        unsigned int ms = us / 1000;
        os_time_dly(ms);
        us = us % 1000;
    }

    /* 对于较短的微秒级延时使用忙等待 */
    if (us) {
        /* 如果延时较长(例如>100us)，可考虑让出CPU */
        if (us > 100) {
            os_time_dly(1); // 让出一个tick的时间片
        } else {
            /* 短延时才使用忙等待 */
            volatile unsigned int i;
            unsigned int loops = us * US_DELAY_LOOP_FACTOR;
            for (i = 0; i < loops; i++) {
                (void)i;
            }
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

/*
 * @brief  进纸检测定时器回调函数
 * @param  arg 回调参数
 * @return none
 * @note   定期检测进纸状态
 */
void Paper_Check_Timer_Callback(void const *arg)
{
    // 进纸判断*******************
    if (gpio_read(ADC_Channel_Paper_Check) < PAPER_THRESHOLD)  // ADC默认值 判断进纸
    {
        PAPER_ON++;
        PAPER_OFF = 0;
    }
    else
    {
        PAPER_ON = 0;
        PAPER_OFF++;
    }
    
    PAPER_ON > PAPER_COUNT_THRESHOLD ? (PAPER_Key = 1) : (PAPER_Key);
    PAPER_OFF > PAPER_COUNT_THRESHOLD ? (PAPER_Key = 0) : (PAPER_Key);
    
    // 如果没有纸张，禁用电机和打印头
    if (PAPER_Key != 1) {
        // 禁用电机和打印头
        MOTO_EN = DISABLE;
        TPH_EN = DISABLE;
    } else {
        // 打印头和电机正常
        MOTO_EN = ENABLE;
        TPH_EN = ENABLE;
    }
}

/* @brief  初始化进纸检测定时器 */
void Init_Paper_Check_Timer(void)
{
    au_id.paper_check_timer_id = sys_timer_add(NULL, Paper_Check_Timer_Callback, 1000); // 1s
}

/*
 * @brief  停止进纸检测定时器
 * @param  none
 * @return none
 * @note   停止进纸检测定时器
 */
void Stop_Paper_Check_Timer(void)
{
    if (au_id.paper_check_timer_id != 0) {
        sys_timer_del(au_id.paper_check_timer_id);
        au_id.paper_check_timer_id = 0;
    }
}