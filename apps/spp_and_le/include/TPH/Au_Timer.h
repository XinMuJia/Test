#ifndef AU_TIMER_H
#define AU_TIMER_H

#include "typedef.h"
#include "system/includes.h"
#include "TPH/Au_Config.h"

// 全局变量声明
extern bool State_Timeout;
extern bool Printer_Timeout;

// 定时器ID结构体
struct Au_ID
{
    u16 state_timer_id;
    u16 printer_timer_id;
    u16 paper_check_timer_id;
};

// 进纸检测相关变量
#define PAPER_THRESHOLD 200//550
#define PAPER_COUNT_THRESHOLD 100

extern struct Au_ID au_id;

// 函数声明
void State_Timer_Callback(void const * arg);
void Init_Timer(void);
bool Get_State_Timeout(void);
void Clean_State_Timeout(void);
void Printer_Timer_Callback(void const * arg);
void Open_Printer_Timeout_Timer(void);
void Close_Printer_Timeout_Timer(void);
bool Get_Printer_Timeout_Status(void);
void us_delay_us(unsigned int us);
void TPH_Check(void);
void TPH_Space(u16	x);
void Moto_Check(void);
// 进纸检测定时器
void Paper_Check_Timer_Callback(void const *arg);

// 定时器3初始化
void timer3_init(u32 usec);
void timer2_init(u32 usec);

#endif // AU_TIMER_H