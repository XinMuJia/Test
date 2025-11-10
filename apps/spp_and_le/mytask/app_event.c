#include "system/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "mytask/app_event.h"
#include "iokey.h"
#include "mytask/app_mytask.h"
#include "mytask/app_time.h"
#include "asm/power/p33.h"
#include "mytask/lcd_ctrl.h"
#include "event.h"
#include "mytask/app_peripheral.h"
#include "TPH/Au_Config.h"


// 添加日志系统定义
#define LOG_TAG_CONST       APP
#define LOG_TAG             "[APP_EVEN]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

// 添加任务定义
#define APP_TASK_NAME "app_task"
extern volatile u8 poweron_detected;
extern u8 lcd_init_complete;

/* 气泵配置   */
#define IO_PORTR_00   0
#define IO_PORTR_01   1
#define OUT_LOW       0
#define OUT_HIGH      1

/* Pump 状态定义 */
struct Node
{
    u8 loop_sel;
    u8 pump_state; // 0: 关闭, 1: 打开
    u32 press_time;
    u32 seconds;
    u16 id;
} Pump_State;




void handle_long_press(u8 key_value)
{
    // 根据按键值执行相应的长按操作
    switch (key_value)
    {
    case 1: // 开机处理
        #if TCFG_POWER_ON_NEED_KEY
        log_info("+");
        if(!poweron_detected) {
            set_key_poweron_flag(1); 

            // 开机
            rtc_port_pr_out(IO_PORTR_01, OUT_HIGH);

            log_info("is poweron");
            poweron_detected = 1;
        } else { // 关机
            log_info("-");
            set_key_poweron_flag(0); 
            rtc_port_pr_out(IO_PORTR_01, OUT_LOW);
            poweron_detected = 0;
        }
        #endif
        break;
    
    default:
        break;
    }
}

void pump_time_update()
{
    Pump_State.seconds++;
    // 气泵运行时，每秒更新一次时间显示
        if(Pump_State.pump_state) {
            LCD_Show_Number_Safe(1, 6, Pump_State.seconds, LCD_CONTENT_TIME);

            // 超过60s自动退出
            if(Pump_State.seconds > 60) {
                Pump_State.pump_state = 0;
                Pump_State.seconds = 0;
                gpio_direction_output(IO_PORTB_02, Pump_State.pump_state);
                sys_timer_del(Pump_State.id); // 停止定时器
                // 设置PR1为数字IO模式
                rtc_port_pr_out(IO_PORTR_00, OUT_LOW);
                
                // 等待LCD空闲
                while (lcd_is_printing()) {
                    os_time_dly(10);
                }
                
                LCD_Clean_Safe();
                LCD_Show_String_Safe(0, 0, "Pump: OFF", LCD_CONTENT_NONE);
                // LCD_Show_String_Safe(1, 0, "Time: 60s", LCD_CONTENT_TIME);
            }
        }
}

void handle_single_click(u8 key_value)
{
    switch (key_value)
    {
        case 5:
            LCD_Clean_Safe();
            // 切换气泵状态
            Pump_State.pump_state = !Pump_State.pump_state; // 0: 关闭, 1: 打开
            // gpio_direction_output(IO_PORTB_02, Pump_State.pump_state); // 控制气泵开关

            if(Pump_State.pump_state) { 
                rtc_port_pr_out(IO_PORTR_00, OUT_HIGH); // 关闭电磁阀
                LCD_Show_String_Safe(0, 0, "Pump: ON", LCD_CONTENT_PUMP_ON);
                LCD_Show_String_Safe(1, 0, "Time: 0 s", LCD_CONTENT_TIME);
                Pump_State.id = sys_timer_add(NULL, pump_time_update, 1000); // 1s周期更新
            } else {
                sys_timer_del(Pump_State.id); // 停止定时器
                LCD_Show_String_Safe(0, 0, "Pump: OFF", LCD_CONTENT_NONE);

                rtc_port_pr_out(IO_PORTR_00, OUT_HIGH); // 打开电磁阀
                os_time_dly(200); // 等待电磁阀关闭2s
                rtc_port_pr_out(IO_PORTR_00, OUT_LOW);
            }
            
            Pump_State.seconds = 0; // 重置计时器
            break;

        case 4:
            // 关闭气泵(通过气压开关判断)
            if(Pump_State.pump_state) { // 只有在开启状态下才响应关闭
                Pump_State.pump_state = 0;
                gpio_direction_output(IO_PORTB_02, Pump_State.pump_state);
                sys_timer_del(Pump_State.id); // 停止定时器
                rtc_port_pr_out(IO_PORTR_00, OUT_LOW);

                // 等待LCD空闲
                while (lcd_is_printing()) {
                    os_time_dly(10);
                }
                
                LCD_Clean_Safe();
                LCD_Show_String_Safe(0, 0, "Pump: OFF", LCD_CONTENT_NONE);
            }
            break;
    }


}

void handle_double_click(u8 key_value)
{
    // 根据按键值执行相应的双击操作
}

void handle_hold(u8 key_value)
{
    // 根据按键值执行相应的持续长按操作
}

void handle_key_release(u8 key_value)
{
    // 根据按键值执行相应的按键释放操作
}


// 按键事件处理函数
void key_event_handler(struct sys_event *event)
{
    // 添加喂狗操作，防止处理过程中看门狗超时
    // clr_wdt();

    printf("Key event detected:\n");
    // local_irq_disable();  // 禁用中断
    if (event->type == SYS_KEY_EVENT) {
        printf("Key_event:\n");
        // 确保是按键事件
        if (event->u.key.init) {
            printf("Key event detected:\n");
            printf("  Key value: %d\n", event->u.key.value);
            printf("  Key type: %d\n", event->u.key.type);
            printf("  Key event: %d\n", event->u.key.event);
            
            // 根据不同的按键事件进行处理
            switch (event->u.key.event) {
                case KEY_EVENT_CLICK:
                    printf("Single click detected\n");
                    // 处理单击事件
                    handle_single_click(event->u.key.value);
                    break;
                    
                case KEY_EVENT_DOUBLE_CLICK:
                    printf("Double click detected\n");
                    // 处理双击事件
                    handle_double_click(event->u.key.value);
                    break;
                    
                case KEY_EVENT_LONG:
                    printf("Long press detected\n");
                    // 处理长按事件
                    handle_long_press(event->u.key.value);
                    break;
                    
                case KEY_EVENT_HOLD:
                    printf("Hold detected\n");
                    // 处理持续按住事件
                    handle_hold(event->u.key.value);
                    break;
                    
                case KEY_EVENT_UP:
                    printf("Key released\n");
                    // 处理按键释放事件
                    handle_key_release(event->u.key.value);
                    break;
                    
                default:
                    break;
            }
        }
    }
    // local_irq_enable();   // 重新启用中断
}

// 在系统初始化时注册事件处理函数
void app_main_init(void)
{
    // 注册按键事件处理
    register_sys_event_handler(SYS_KEY_EVENT, DEVICE_EVENT_FROM_KEY, 0, key_event_handler);
    // sys_hi_timer_add(NULL, key_event_handler, 50); // 每100ms检查一次按键事件(硬件定时器)
}