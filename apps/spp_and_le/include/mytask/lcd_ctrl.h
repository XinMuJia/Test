#ifndef _LCD_CTRL_H_
#define _LCD_CTRL_H_
#include "system/includes.h"

// LCD显示状态定义
typedef enum {
    LCD_STATE_IDLE = 0,        // 空闲状态，非打印
    LCD_STATE_PRINTING,        // 打印状态
    LCD_STATE_UPDATING,        // 更新状态
    LCD_STATE_CLEANING         // 清屏状态
} lcd_state_t;

// LCD显示内容定义
typedef enum {
    LCD_CONTENT_NONE = 0,      // 无内容
    LCD_CONTENT_PUMP_ON,       // 气泵开启
    LCD_CONTENT_PUMP_OFF,      // 气泵关闭
    LCD_CONTENT_BATTERY,       // 电量显示
    LCD_CONTENT_TIME           // 时间显示(计时)
} lcd_content_t;

// 外部声明
extern volatile lcd_state_t lcd_current_state;
extern volatile lcd_content_t lcd_current_content;
extern u8 lcd_init_complete;

// 状态机声明函数声明
void lcd_set_state(lcd_state_t new_state);
lcd_state_t lcd_get_state(void);
void lcd_set_content(lcd_content_t content);
lcd_content_t lcd_get_content(void);
u8 lcd_is_printing(void);
u8 lcd_can_print(void);

// 函数声明
void LCD1602_GPIO_Init_Out(void);
void LCD1602_Init(void);
void LCD_Write_Command(u8 cmd);
void LCD_Write_Data(u8 data);
void LCD_Set_Cursor(u8 x, u8 y);
void LCD_Show_String(u8 x, u8 y, const char *str);
void LCD_Show_Char(u8 x, u8 y, u8 data);
void LCD_Show_Number(u8 x, u8 y, u32 num);
void LCD_Show_Num(u8 x, u8 y, u8 num);
void LCD_Clear(void);
void LCD_Clean(void);

// 状态机使用的函数声明
void LCD_Show_String_Safe(u8 x, u8 y, const char *str, lcd_content_t content_type);
void LCD_Clean_Safe(void);
void LCD_Show_Number_Safe(u8 x, u8 y, u32 num, lcd_content_t content_type);
#endif  //_LCD_CTRL_H_