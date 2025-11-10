#include "system/includes.h"
#include "app_config.h"
#include "mytask/lcd_ctrl.h"

// 添加日志系统定义
#define LOG_TAG_CONST       APP
#define LOG_TAG             "[LCD_CTRL]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"

// GPIO引脚定义
#define LCD_RS_PIN    IO_PORTA_07   // 寄存器选择
#define LCD_RW_PIN    IO_PORTA_08   // 读写选择
#define LCD_EN_PIN    IO_PORTA_09   // 使能信号
#define LCD_D4_PIN    IO_PORTC_01   // 数据位4
#define LCD_D5_PIN    IO_PORTA_15   // 数据位5  
#define LCD_D6_PIN    IO_PORTC_02   // 数据位6
#define LCD_D7_PIN    IO_PORTC_03   // 数据位7

u8 lcd_init_complete;
// LCD互斥锁
static OS_MUTEX lcd_mutex;
static OS_MUTEX lcd_state_mutex;    // LCD状态锁

// 延时函数
static void lcd_delay_ms(u32 msec)
{
    os_time_dly(msec);
}

static void lcd_delay_us(u32 usec)
{
    volatile u32 i;
    for (i = 0; i < usec * 48; i++) {
        asm("nop");
    }
}

// LCD1602端口初始化,配置为输出
void LCD1602_GPIO_Init_Out(void)
{
    // 初始化LCD控制引脚为输出模式
    gpio_direction_output(LCD_RS_PIN, 0);
    gpio_direction_output(LCD_RW_PIN, 0);  // 写模式
    gpio_direction_output(LCD_EN_PIN, 0);
    gpio_direction_output(LCD_D4_PIN, 0);
    gpio_direction_output(LCD_D5_PIN, 0);
    gpio_direction_output(LCD_D6_PIN, 0);
    gpio_direction_output(LCD_D7_PIN, 0);
    
    printf("LCD1602 GPIO_Init_ComPlete\n");
}

// 写入4位数据
static void lcd_write_nibble(u8 data, u8 rs)
{
    // 设置RS引脚
    gpio_write(LCD_RS_PIN, rs);
    
    // 设置数据位
    gpio_write(LCD_D4_PIN, (data >> 4) & 0x01);
    gpio_write(LCD_D5_PIN, (data >> 5) & 0x01);
    gpio_write(LCD_D6_PIN, (data >> 6) & 0x01);
    gpio_write(LCD_D7_PIN, (data >> 7) & 0x01);
    
    // 使能脉冲
    gpio_write(LCD_EN_PIN, 1);
    lcd_delay_us(10);
    gpio_write(LCD_EN_PIN, 0);
    lcd_delay_us(100);
}

// 写入完整8位数据
static void lcd_write_byte(u8 data, u8 rs)
{
    lcd_write_nibble(data, rs);           // 高4位
    lcd_write_nibble(data << 4, rs);      // 低4位
}

// 用于写指令 - 对应原LCD1602_WriteCmd
void LCD_Write_Command(u8 cmd)
{
    lcd_write_byte(cmd, 0); // rs=0 表示写命令
    lcd_delay_ms(2); // 命令执行时间
}

// 用于写数据 - 对应原LCD1602_WriteDate  
void LCD_Write_Data(u8 data)
{
    lcd_write_byte(data, 1); // rs=1 表示写数据
    lcd_delay_ms(2);
}

// 设置显示地址 - 对应原LCD1620_SetAddress
void LCD_Set_Cursor(u8 x, u8 y)
{
    u8 address;
    if (x == 0) {
        address = 0x80 + y;  // 第一行
    } else {
        address = 0xC0 + y; // 第二行
    }

    LCD_Write_Command(address);
}

// LCD1602显示字符串
void LCD_Show_String(u8 x, u8 y, const char *str)
{
    os_mutex_pend(&lcd_mutex, 0);

    LCD_Set_Cursor(x, y);  // 添加设置光标位置
    while (*str != '\0') {
        LCD_Write_Data(*str++);
    }

    os_mutex_post(&lcd_mutex);
}

// LCD1602显示字符
void LCD_Show_Char(u8 x, u8 y, u8 data)
{
    os_mutex_pend(&lcd_mutex, 0);

    LCD_Set_Cursor(x, y);
    LCD_Write_Data(data);

    os_mutex_post(&lcd_mutex);
}

// LCD1602显示数字
void LCD_Show_Number(u8 x, u8 y, u32 num)
{
    os_mutex_pend(&lcd_mutex, 0);

    char buffer[10];
    sprintf(buffer, "%lu", num);
    
    // LCD_Set_Cursor(x, y);
    LCD_Show_String(x, y, buffer);

    os_mutex_post(&lcd_mutex);
}

// 清屏函数
void LCD_Clear(void)
{
    LCD_Write_Command(0x01); // 清屏命令
    lcd_delay_ms(10); // 清屏需要较长时间
}

// 液晶初始化函数 - 对应原LCD1602_Init（改为4位模式）
void LCD1602_Init(void)
{
    printf(">>>>>>>>>>>>>>>>>LCD1602...\n");
    // 创建互斥锁
    os_mutex_create(&lcd_mutex);
    os_mutex_create(&lcd_state_mutex);
    
    // 初始化状态
    lcd_set_state(LCD_STATE_IDLE);
    lcd_set_content(LCD_CONTENT_NONE);
    
    // 初始化GPIO
    LCD1602_GPIO_Init_Out();
    
    // 等待LCD上电稳定
    lcd_delay_ms(50);

    // 4位模式初始化序列
    lcd_write_nibble(0x30, 0); // 步骤1
    lcd_delay_ms(5);
    lcd_write_nibble(0x30, 0); // 步骤2
    lcd_delay_ms(5);
    lcd_write_nibble(0x30, 0); // 步骤3
    lcd_delay_ms(5);
    lcd_write_nibble(0x20, 0); // 切换到4位模式
    lcd_delay_ms(5);

    // 功能设置(4位模式)
    LCD_Write_Command(0x28); // 4位模式, 2行, 5x8字体
    lcd_delay_ms(5);
    
    LCD_Write_Command(0x0C); // 开显示,不显示光标，光标不闪烁
    lcd_delay_ms(5);
    
    LCD_Write_Command(0x06); // 光标加1，屏幕显示不移动
    lcd_delay_ms(5);
    
    LCD_Write_Command(0x01); // 清屏
    lcd_delay_ms(10);
    
    printf("LCD1602ComPlete\n");
    
    // 显示测试信息
    // LCD_Show_String(0, 0, "LCD1602 OK");

    // LCD_Show_String(1, 0, "4-BIT MODE");
    // 显示测试信息
    LCD_Show_String_Safe(0, 0, "LCD1602 OK", LCD_CONTENT_NONE);
    LCD_Show_String_Safe(1, 0, "STATE MACHINE", LCD_CONTENT_NONE);

    lcd_init_complete = 1;
}

// 清屏函数
void LCD_Clean(void)
{
    os_mutex_pend(&lcd_mutex, 0);
    LCD_Clear();
    os_mutex_post(&lcd_mutex);
}

// 简化版显示数字
void LCD_Show_Num(u8 x, u8 y, u8 num)
{
    LCD_Set_Cursor(x, y);
    LCD_Write_Data('0' + (num % 10)); // 显示单个数字
}



/*********************************************************************************************
    *   状态机设置
*********************************************************************************************/
static volatile lcd_state_t lcd_current_state = LCD_STATE_IDLE;
static volatile lcd_content_t lcd_current_content = LCD_CONTENT_NONE;
// static OS_MUTEX lcd_state_mutex;

// 设置LCD状态
void lcd_set_state(lcd_state_t new_state)
{
    os_mutex_pend(&lcd_state_mutex, 0);
    lcd_current_state = new_state;
    os_mutex_post(&lcd_state_mutex);
}

// 获取LCD状态
lcd_state_t lcd_get_state(void)
{
    lcd_state_t state;
    os_mutex_pend(&lcd_state_mutex, 0);
    state = lcd_current_state;
    os_mutex_post(&lcd_state_mutex);
    return state;
}

// 设置显示内容
void lcd_set_content(lcd_content_t content)
{
    os_mutex_pend(&lcd_state_mutex, 0);
    lcd_current_content = content;
    os_mutex_post(&lcd_state_mutex);
}

// 获取显示内容
lcd_content_t lcd_get_content(void)
{
    lcd_content_t content;
    os_mutex_pend(&lcd_state_mutex, 0);
    content = lcd_current_content;
    os_mutex_post(&lcd_state_mutex);
    return content;
}

// 检查是否正在打印
u8 lcd_is_printing(void)
{
    return (lcd_get_state() == LCD_STATE_PRINTING);
}

// 检查是否可以打印
u8 lcd_can_print(void)
{
    lcd_state_t state = lcd_get_state();
    return (state == LCD_STATE_IDLE || state == LCD_STATE_UPDATING);
}

/*********************************************************************************************
    *   状态机使用
*********************************************************************************************/
// LCD显示字符串函数
void LCD_Show_String_Safe(u8 x, u8 y, const char *str, lcd_content_t content_type)
{
    if (!lcd_init_complete || !lcd_can_print()) {
        log_info("LCD busy, skip print");
        return;
    }
    
    lcd_set_state(LCD_STATE_PRINTING);
    lcd_set_content(content_type);
    
    os_mutex_pend(&lcd_mutex, 0);
    LCD_Set_Cursor(x, y);
    while (*str != '\0') {
        LCD_Write_Data(*str++);
    }
    os_mutex_post(&lcd_mutex);
    
    lcd_set_state(LCD_STATE_IDLE);
}

// 清屏函数
void LCD_Clean_Safe(void)
{
    if (!lcd_init_complete) {
        return;
    }
    
    lcd_set_state(LCD_STATE_CLEANING);
    
    os_mutex_pend(&lcd_mutex, 0);
    LCD_Write_Command(0x01);
    lcd_delay_ms(20);
    os_mutex_post(&lcd_mutex);
    
    lcd_set_state(LCD_STATE_IDLE);
    lcd_set_content(LCD_CONTENT_NONE);
}

// 显示数字函数
void LCD_Show_Number_Safe(u8 x, u8 y, u32 num, lcd_content_t content_type)
{
    if (!lcd_init_complete || !lcd_can_print()) {
        return;
    }
    
    lcd_set_state(LCD_STATE_PRINTING);
    lcd_set_content(content_type);
    
    os_mutex_pend(&lcd_mutex, 0);
    char buffer[10];
    sprintf(buffer, "%lu", num);
    LCD_Set_Cursor(x, y);
    LCD_Show_String(x, y, buffer);
    os_mutex_post(&lcd_mutex);
    
    lcd_set_state(LCD_STATE_IDLE);
}


// LCD1602显示字符 (Safe version with state machine and mutex)
void LCD_Show_Char_Safe(u8 x, u8 y, u8 data, lcd_content_t content_type)
{
    // 1. 检查 LCD 是否已初始化完成
    if (!lcd_init_complete || !lcd_can_print()) {
        log_info("LCD busy, skip print");
        return;
    }

    // 2. 设置 LCD 状态为正在打印 (PRINTING)
    lcd_set_state(LCD_STATE_PRINTING);
    // 3. 设置 LCD 当前显示内容的类型
    lcd_set_content(content_type);

    // 4. 获取 LCD 互斥锁，防止多个任务同时访问 LCD 硬件
    os_mutex_pend(&lcd_mutex, 0);

    // 5. 执行实际的 LCD 操作
    LCD_Set_Cursor(x, y);       // 设置光标位置
    LCD_Write_Data(data);       // 写入字符数据

    // 6. 释放 LCD 互斥锁
    os_mutex_post(&lcd_mutex);

    // 7. 操作完成，将 LCD 状态设置回空闲 (IDLE)
    lcd_set_state(LCD_STATE_IDLE);
}