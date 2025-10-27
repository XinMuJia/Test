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


// 添加日志系统定义
#define LOG_TAG_CONST       APP
#define LOG_TAG             "[MY_TASK]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

extern const struct task_info task_info_table[];
// 对应的任务函数实现
u8 poweron_detected = 0;

extern u8 lcd_init_complete;


/* 任务配置   */
#define IO_PORTR_00   0
#define IO_PORTR_01   1
#define OUT_LOW       0
#define OUT_HIGH      1

void rtc_32k_clock_test();
void start_rtc_clock_test();

/* 消息队列 */
void my_task(void *p)
{
    while(1) {
        printf("my_task\n");
        os_time_dly(100);
    }
}

void show_battery_level(void)
{
#if TCFG_SYS_LVD_EN
    static u8 last_percentage = 0xFF;
    u8 percentage_num = 3;
    u8 percentage = get_vbat_percent();
    
    if(percentage > 100) percentage = 100;
    
    // 只在电量变化时更新
    if(percentage != last_percentage) {
        char num_str[4];
        
        // 格式化数字
        if(percentage == 100) {
            sprintf(num_str, "100");
        } else {
            sprintf(num_str, "%2d", percentage); // 两位数字，不足补空格
            percentage_num = 2;
        }
        
        // 只更新数字部分
        for(int i = 0; i < percentage_num; i++) {
            if(num_str[i] != '\0') {
                LCD_Show_Num(1, 4 + (3 - percentage_num + i), num_str[i]);
            }
        }
        LCD_Show_Char(1, 4 + percentage_num + 1, '%'); // 显示百分号
    }
#endif
}


/* 检查开机键   */
// void check_power_on_key()
// {
// #if TCFG_POWER_ON_NEED_KEY
//     u32 delay_10ms_cnt = 0;
//     while (1) {
//         clr_wdt();
//         os_time_dly(1);
//         extern u8 get_power_on_status(void);
//         if (get_power_on_status()) {
//             log_info("+");
//             delay_10ms_cnt++;
            
//             // 连续按键1s以上，按键开机
//             if (delay_10ms_cnt > 100 && !poweron_detected) { //1s
//                 set_key_poweron_flag(1); 

//                 // 开机
//                 gpio_direction_output(IO_PORTB_11, 1);     
//                 gpio_write(IO_PORTB_11, 1);
//                 log_info("is poweron");
//                 delay_10ms_cnt = 0;
//                 poweron_detected = 1;
//                 return;
//                 // 关机检测：开机后继续长按2秒关机
//             } else if (delay_10ms_cnt > 200 && poweron_detected) { 
//                 log_info("Power off key detected\n");
//                 poweron_detected = 0;
//                 delay_10ms_cnt = 0;
//                 power_set_soft_poweroff();
//                 gpio_direction_output(IO_PORTB_11, 0); 
//                 return;
//             }
//         }
//     }
// #endif
// }

// void PowerOnKey_Task(void)
// {
//     while (1) {
//         check_power_on_key();

//         os_time_dly(100);  // 避免任务频繁运行
//     }
// }


/* 循环选择 */
// void app_loop_select(void)
// {
//     u8 loop_sel = 0;
//     u8 pump_state = 0; // 0: 关闭, 1: 打开
//     u32 press_time = 0;
//     u32 seconds = 0;

//     while(!poweron_detected || !lcd_init_complete) {
//         os_time_dly(10); // 等待开机按键检测完成
//     }

//     /* 基础配置 */
//     gpio_direction_input(IO_PORTB_04); // 配置为输入
//     gpio_direction_output(IO_PORTB_02, 0); // 初始化气泵为关闭状态

//     // 设置PR1为数字IO模式
//     p33_tx_1byte(R3_OSL_CON, 0);
//     rtc_port_pr_die(IO_PORTR_00, 1);

//     /* 选择APP */
//     while (1) {
//         // loop_sel = get_check_status() - 1; 
//         loop_sel = io_get_key_value();//用于读取按键状态选择APP
//         log_info("APP_LOOP_SEL: %d", loop_sel);
//         switch (loop_sel) {
//         case 4:
//             LCD_Clean_Safe();
//             // 切换气泵状态
//             pump_state = !pump_state;
//             if(pump_state) { // 次序问题
//                 rtc_port_pr_out(IO_PORTR_00, OUT_HIGH);
//                 LCD_Show_String_Safe(0, 0, "Pump: ON", LCD_CONTENT_PUMP_ON);
//                 LCD_Show_String_Safe(1, 0, "Time: 0 s", LCD_CONTENT_TIME);
//             } else {
//                 rtc_port_pr_out(IO_PORTR_00, OUT_LOW);
//                 LCD_Show_String_Safe(0, 0, "Pump: OFF", LCD_CONTENT_NONE);
//             }

//             gpio_direction_output(IO_PORTB_02, pump_state);
//             press_time = 0; // 重置计时器
//             break;

//         case 5:
//             // 关闭气泵(通过气压开关判断)
//             if(pump_state) { // 只有在开启状态下才响应关闭
//                 pump_state = 0;
//                 gpio_direction_output(IO_PORTB_02, pump_state);
//                 rtc_port_pr_out(IO_PORTR_00, OUT_LOW);

//                 // 等待LCD空闲
//                 while (lcd_is_printing()) {
//                     os_time_dly(10);
//                 }
                
//                 LCD_Clean_Safe();
//                 LCD_Show_String_Safe(0, 0, "Pump: OFF", LCD_CONTENT_NONE);
//             }
//             break;
//         }

//         // 气泵运行时，每秒更新一次时间显示
//         if(pump_state) {
//             press_time++;
//             // 每1000ms(10个100ms周期)更新一次LCD显示
//             if(press_time % 10 == 0) {
//                 seconds = press_time / 10;
//                 LCD_Show_Number_Safe(1, 6, seconds, LCD_CONTENT_TIME);
//             }
            
//             // 超过60s自动退出
//             if(seconds > 60) {
//                 pump_state = 0;
//                 press_time = 0;
//                 seconds = 0;
//                 gpio_direction_output(IO_PORTB_02, pump_state);

//                 // 设置PR1为数字IO模式
//                 rtc_port_pr_out(IO_PORTR_00, OUT_LOW);

//                 // 等待LCD空闲
//                 while (lcd_is_printing()) {
//                     os_time_dly(10);
//                 }
                
//                 LCD_Clean_Safe();
//                 LCD_Show_String_Safe(0, 0, "Pump: OFF", LCD_CONTENT_NONE);
//                 // LCD_Show_String_Safe(1, 0, "Time: 60s", LCD_CONTENT_TIME);
//             }
//         }
//         os_time_dly(10);
//     }
// }


void Lcd_Task(void)
{
    /* 等待开机完成再初始化 LCD */
    while (!poweron_detected) {
        os_time_dly(10);
    }

    init_software_rtc(); // 初始化软件RTC
    log_info("LCD1602 Init...");
    LCD1602_Init();

    u8 last_content = 0xFF;

    // 设置系统时间
    struct sys_time new_time = {0};
    new_time.year = 2025;
    new_time.month = 1;
    new_time.day = 1;
    new_time.hour = 15;
    new_time.min = 13;
    new_time.sec = 0;
    set_system_time(&new_time);
    
    while (1) {
        /* 只在 LCD 空闲时更新显示 */
        if (lcd_get_state() == LCD_STATE_IDLE) {
            u8 content = lcd_get_content();

            if (content == LCD_CONTENT_NONE || content == LCD_CONTENT_BATTERY) {
                /* 仅在从其他内容切换回来或首次进入时清屏并显示固定文本 */
                if (content != last_content) {
                    LCD_Clean_Safe();
                    LCD_Show_String_Safe(1, 0, "Bat:  ", LCD_CONTENT_BATTERY);

                    last_content = LCD_CONTENT_BATTERY;
                }

                 // 显示实时时间
                struct sys_time current_time;
                get_current_time(&current_time);
                
                // 格式化时间字符串 "HH:MM:SS"
                char time_str[9];
                sprintf(time_str, "%02d:%02d:%02d", 
                        current_time.hour, 
                        current_time.min, 
                        current_time.sec);

                // 在第一行显示时间
                LCD_Show_String(0, 0, time_str);

                /* 更新电量（show_battery_level 内部已做变化判断）*/
                show_battery_level();
            } else {
                /* 如果当前不是电量页，保持 last_content 与实际同步，避免误判 */
                last_content = content;
            }
        }

        /* 适当休眠，降低 CPU 占用 */
        os_time_dly(100);
    }
}

void Task_Init(void)
{
    /* os_task_create(app_main_task, (void *)0, TASK_APP_MAIN_NAME); */
    // 创建检测开机按键线程，使用任务表配置参数
    // os_task_create(key_event_handler ,
    //               (void *)0,
    //               task_info_table[9].prio,
    //               task_info_table[9].stack_size,
    //               0,
    //               task_info_table[9].name);
    // os_task_create(my_task, (void *)0, "my_task", 4, 256, 0); // 创建自定义任务线程
    os_task_create( my_task,
                    (void *)0,
                    task_info_table[8].prio,
                    task_info_table[8].stack_size,
                    0,
                    task_info_table[8].name);

    os_task_create( Lcd_Task,
                    (void *)0,
                    task_info_table[10].prio,
                    task_info_table[10].stack_size,
                    0,
                    task_info_table[10].name);

    // os_task_create( app_loop_select,
    //                 (void *)0,
    //                 task_info_table[11].prio,
    //                 task_info_table[11].stack_size,
    //                 0,
    //                 task_info_table[11].name);
    
    task_info_add(task_info_table);   
    log_info("Task_Init Over");
}