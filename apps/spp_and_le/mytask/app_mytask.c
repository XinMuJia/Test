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
#include "TPH/Au_Motor.h"
#include "TPH/Au_Printf.h"
#include "TPH/Au_Adc.h"
#include "mytask/app_queue.h"


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
volatile u8 poweron_detected = 0;

extern u8 lcd_init_complete;
extern bool Printer_Timeout;
extern u8 percentage;

// 长度
u8 	TPH_LAN_TEST;	

/* 任务配置   */
#define IO_PORTR_00   0
#define IO_PORTR_01   1
#define OUT_LOW       0
#define OUT_HIGH      1

extern FunctionalState	MOTO_EN_It;
extern FunctionalState	MOTO_EN;
BitAction	    PAPER_Key;
/* 打印 */
void my_task(void *p)
{
    /* 等待开机完成再初始化 LCD */
    while (!poweron_detected) {
        os_time_dly(10);
    }

    while(1) {
        u32 i = get_adc_level(ADC_Channel_Paper_Check, Bit_RESET);
        u32 j = get_adc_level(ADC_Channel_Paper_Check2, Bit_RESET);
        printf("ADC_Num = %d\n %d\n", i, j);
        os_time_dly(100);
    }
}

void show_battery_level(u8 is_simple)
{
#if TCFG_SYS_LVD_EN
    static u8 last_percentage = 0xFF;
    u8 percentage_num = 3;
    u8 percentage = get_vbat_percent();
    printf("vbat==%d\n", percentage);
    if(percentage > 100) percentage = 100;
    
    // 只在电量变化或者页面重新加载时更新
    if(percentage != last_percentage || is_simple) {
        // LCD_Show_String(1, 5, "    "); // 清理4个字符位置
        LCD_Show_String_Safe(1, 5, "    ", LCD_CONTENT_BATTERY);
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
            // 从右到左显示，固定右侧对齐
            // LCD_Show_Char(1, 6 - (percentage_num - 1 - i), num_str[i]);
            LCD_Show_Char_Safe(1, 6 - (percentage_num - 1 - i), num_str[i], LCD_CONTENT_BATTERY);
        }
    }
        LCD_Show_Char_Safe(1, 7, '%', LCD_CONTENT_BATTERY); // 百分号固定在最右侧

        last_percentage = percentage;
    }
#endif
}


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
    u8 is_simple = 0;
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
            is_simple = (content != last_content);
            if (content == LCD_CONTENT_NONE || content == LCD_CONTENT_BATTERY) {
                /* 仅在从其他内容切换回来或首次进入时清屏并显示固定文本 */
                if (is_simple) {
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
                // LCD_Show_String(0, 0, time_str);
                LCD_Show_String_Safe(0, 0, time_str, LCD_CONTENT_BATTERY);

                /* 更新电量（show_battery_level 内部已做变化判断）*/
                show_battery_level(is_simple);
            } else {
                /* 如果当前不是电量页，保持 last_content 与实际同步，避免误判 */
                last_content = content;
            }
            is_simple = 0;
        }

        /* 适当休眠，降低 CPU 占用 */
        os_time_dly(10);
    }
}

// 创建一个任务，用于打印数据
void Print_Task(void)
{
    // clr_wdt(); // 清除看门狗  

    char ble_data[256];
    uint16_t data_length;
    uint16_t data_handle;
    u8 print_data_flag = 1;

    /* 等待开机完成再初始化 LCD */
    while (!poweron_detected) {
        os_time_dly(10);
    }

    Init_Printer();
    while (1) {
        // 测试打印
        if(print_data_flag) {
            TPH_Start();
            for(int lps=64;lps>0;lps--)
            {
                TPH_Loop2();
            }
            TPH_PrintString(0,": ; < = > ? @ A B C D E F G H I J K L M N O P Q",24);
            print_data_flag = 0;
            TPH_Esc();
        }

        // 打印BLE数据
        if(ble_data_dequeue(ble_data, &data_length, &data_handle)) {
            ble_data[data_length] = '\0';
            printf("ble_data_dequeue: %d\n", data_length);
            printf("ble_data_dequeue: %s\n", ble_data);
            TPH_Start();
            TPH_PrintString(0, ble_data, data_length);
            TPH_Esc();
            memset(ble_data, 0, sizeof(ble_data));
        }

        os_time_dly(100);
    }
}

void Task_Init(void)
{
    clr_wdt();
    /* os_task_create(app_main_task, (void *)0, TASK_APP_MAIN_NAME); */
    // 创建检测开机按键线程，使用任务表配置参数
    os_task_create( Print_Task ,
                    (void *)0,
                    task_info_table[9].prio,
                    task_info_table[9].stack_size,
                    0,
                    task_info_table[9].name);

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