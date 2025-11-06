#include "system/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "update.h"
#include "update_loader_download.h"
#include "app_charge.h"
#include "app_power_manage.h"
#include "asm/charge.h"
#include "asm/power/p33.h"
#include "mytask/lcd_ctrl.h"
#include "mytask/app_time.h"
#include "rtc_alarm.h"
#include "iokey.h"
#include "mytask/app_event.h"
#include "TPH/Au_Timer.h"
#include "TPH/Au_Start.h"

/* 气泵配置   */
#define IO_PORTR_00   0
#define IO_PORTR_01   1
#define OUT_LOW       0
#define OUT_HIGH      1

/* 气泵初始化 */
void Pump_Init(void)
{
    gpio_direction_input(IO_PORTB_04); // 配置为输入
    gpio_direction_output(IO_PORTB_02, 0); // 初始化气泵为关闭状态

    // 设置PR1为数字IO模式
    p33_tx_1byte(R3_OSL_CON, 0);
    rtc_port_pr_die(IO_PORTR_00, 1);
    rtc_port_pr_die(IO_PORTR_01, 1);
}

void Peripheral_Init(void)
{
    UpStart();
    Pump_Init();
    timer3_init(200); // 初始化定时器3，200us中断
}