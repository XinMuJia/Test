#include "typedef.h"
#include "TPH/Au_Config.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "update.h"
#include "TPH/Au_Timer.h"
#include "TPH/Au_Motor.h"
#include "TPH/Au_Printf.h"
#include "time.h"
#include "TPH/Au_Adc.h"

// 电机和打印头使能标志
FunctionalState	MOTO_EN_It	= 	FUN_DISABLE;
FunctionalState	TPH_EN_It	=	FUN_DISABLE;
FunctionalState	TPH_EN;
FunctionalState	MOTO_EN;

// extern	u16 			Detect_USART[6];
extern 	u16 			UpStartUSART[14];
extern  BitAction	    PAPER_Key;
extern 	u8				res;
extern 	u8				NHeatTime;														//加热间隔时间
extern	BitAction	    STB_ON,STB_OFF;

bool State_Timeout = false;
bool Printer_Timeout = false;
u8 time1_test_flag = 0;
u8	cis=0,STB_L_Num=0,STB_H_Num=0; // 打印头通道数
u16	It_Num;
u32	It_Loop=0;
u8 PAPER_ON=0,PAPER_OFF=0;

// 电压百分比
u8 percentage=0;

// ADC检测
extern u16	 ADC_Num[3];

struct Au_ID au_id;

#ifndef US_DELAY_LOOP_FACTOR
#define US_DELAY_LOOP_FACTOR    10u    // 经验值，越大忙等待越长，需校准
#endif

/*  定时器中断 */
#define USER_TIMER 		JL_TIMER3//微妙延时使用的定时器 地址 
#define USER_TIMER_IRQ 	IRQ_TIME3_IDX//微妙延时使用的定时器 中断号
___interrupt static void timer_isr()
{
    USER_TIMER->CON |= BIT(14); //清pending

    // 进纸检测定时器回调
    Paper_Check_Timer_Callback(NULL);

    // 打印头控制
    TPH_Check();
}

___interrupt static void timer2_isr()
{
    // 清除中断标志位 (Pending bit)
    JL_TIMER2->CON |= BIT(14); 

    // 电机控制
    Moto_Check();
}

/*
 * @brief  初始化定时器2
 * @param  usec: 定时周期，单位为微秒
 * @return none
 * @note   配置定时器2以指定微秒为周期产生中断
 *         定时器2的寄存器地址为 JL_TIMER2，中断号为 IRQ_TIME2_IDX
 */
void timer2_init(u32 usec)
{
    // 获取定时器时钟频率
    u32 timer_clk = clk_get("timer");
    
    // 计算周期值: 
    // 定时器时钟源经过4分频，所以实际频率为timer_clk/4
    // 每个计数周期的时间为 4/timer_clk 秒
    u32 prd = (u64)usec * (timer_clk / 4) / 1000000;
    
    // 限制PRD值在16位范围内
    if (prd > 65535) {
        prd = 65535; // 最大值
    } else if (prd == 0) {
        prd = 1; // 最小值
    }
    
    // 先禁用定时器
    JL_TIMER2->CON = 0;
    
    // 清除pending位
    JL_TIMER2->CON = BIT(14); 
    // 清零计数器
    JL_TIMER2->CNT = 0;       
    // 设置周期值
    JL_TIMER2->PRD = prd;     
    
    // 配置定时器但不使能: 时钟源+分频 (4分频)
    // BIT(3): 选择晶振作为时钟源 (具体位定义需参考芯片手册)
    // BIT(4): 4 分频
    JL_TIMER2->CON = BIT(3)|BIT(4);  

    // 注册中断处理函数
    request_irq(IRQ_TIME2_IDX, 3, timer2_isr, 0);
    
    // 最后使能定时器 (BIT(0): 定时计数模式使能)
    JL_TIMER2->CON |= BIT(0);
}


// void timer3_init(u32 usec)
// {
// 	static u32 prd = 0;
// 	//prd = clk_get("timer")/4000000 * usec;
// 	USER_TIMER->CON = BIT(14);//清pending
//     USER_TIMER->CNT = 0;
// 	USER_TIMER->PRD = 580;//65535 == 11ms 596 == 100us
// 	request_irq(USER_TIMER_IRQ, 3, timer_isr, 0);
// 	USER_TIMER->CON = BIT(0)|BIT(3)|BIT(4);//BIT(0)定时计数模式 BIT(3):晶振为时钟源 BIT(4):4 分频
// }

// 指定传入的微秒数，初始化定时器3
void timer3_init(u32 usec)
{
    // 获取定时器时钟频率
    u32 timer_clk = clk_get("timer");
    
    // 计算周期值: 
    // 定时器时钟源经过4分频，所以实际频率为timer_clk/4
    // 每个计数周期的时间为 4/timer_clk 秒
    // 要实现usec微秒定时，需要的计数次数为: usec * (timer_clk/4) / 1000000
    // 简化为: (usec * timer_clk) / 4000000
    // u32 prd = (timer_clk / 4000000) * usec; // 计算周期值，防止溢出
    u32 prd = (u64)usec * (timer_clk / 4) / 1000000;
    
    // 限制PRD值在16位范围内
    if (prd > 65535) {
        prd = 65535; // 最大值
    } else if (prd == 0) {
        prd = 1; // 最小值
    }
    // 先禁用定时器
    USER_TIMER->CON = 0;
    
    USER_TIMER->CON = BIT(14); // 清除pending位
    USER_TIMER->CNT = 0;       // 清零计数器
    USER_TIMER->PRD = prd;     // 设置周期值
    
    // 配置定时器但不使能
    USER_TIMER->CON = BIT(3)|BIT(4);  // 时钟源+分频

    request_irq(USER_TIMER_IRQ, 3, timer_isr, 0);
    
    // // 配置定时器: 定时计数模式 | 晶振为时钟源 | 4分频
    // USER_TIMER->CON = BIT(0)|BIT(3)|BIT(4);

    // 最后使能定时器
    USER_TIMER->CON |= BIT(0);
}

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
 * @brief  进纸检测定时器回调函数
 * @param  arg 回调参数
 * @return none
 * @note   定期检测进纸状态
 */
void Paper_Check_Timer_Callback(void const *arg)
{
    u32 adc_value = get_adc_level(ADC_Channel_Paper_Check, Bit_RESET);

    // 增加有效性检查
    if(adc_value == 0) {
        // 可能是采样未完成，暂时忽略本次采样
        return;
    }
    

    // 进纸判断*******************
    if (adc_value < PAPER_THRESHOLD)  // ADC默认值 判断进纸
    {
        // get_adc_level(ADC_Channel_Paper_Check, Bit_RESET)
        // adc_get_value(ADC_Channel_Paper_Check)
        PAPER_ON++;
        PAPER_OFF = 0;
    }
    else
    {
        PAPER_ON = 0;
        PAPER_OFF++;
    }
    
    PAPER_ON > PAPER_COUNT_THRESHOLD ? (PAPER_Key = Bit_SET) : (PAPER_Key);
    PAPER_OFF > PAPER_COUNT_THRESHOLD ? (PAPER_Key = Bit_RESET) : (PAPER_Key);
    PAPER_Key!=Bit_SET?MOTO_EN=DISABLE,TPH_EN=DISABLE:(PAPER_Key);
}


/*
  * @brief  打印头控制函数
  * @param  none
  * @return none	
  * @note   根据使能标志控制打印头工作
 */
void TPH_Check(void)
{
    if (TPH_EN == FUN_ENABLE && TPH_EN_It == FUN_ENABLE) {
        /* 限制阈值在 0..10 之间以避免溢出 */
        u8 low_limit = (UpStartUSART[1] > 10) ? 10 : (u8)UpStartUSART[1];
        u8 high_limit = 10 - low_limit;

        /* 打印头控制逻辑 高低电平形成方波*/
        if (STB_H_Num <= high_limit) {
            Digital_Write(STB1_Pin, STB_ON);
            STB_H_Num++;
        } else {
            Digital_Write(STB1_Pin, STB_OFF);
            if (++STB_L_Num > low_limit) {
                STB_H_Num = 0;
                STB_L_Num = 0;
            }
        }
    } else {
        Digital_Write(STB1_Pin, Bit_SET);
        TPH_EN_It = FUN_DISABLE;
    }
}


void Moto_Check(void)
{
    // 步进电机驱动逻辑
    if (MOTO_EN == FUN_ENABLE && MOTO_EN_It == FUN_ENABLE) 
    {
        if (It_Num >= UpStartUSART[2])          // 达到步进频率时执行步进
        {
            res &= 0x07;                        // 保持res在0-7范围内
            MOTOR_STEP(step_sequence[res]);     // 执行步进动作
            It_Num = 0;                         // 重置计时
            res++;                              // 准备下一步
            cis++;                              // 步数计数
        }
    }

    // 完成4步后停止中断任务
    if (cis == 4)
    {
        MOTO_EN_It = FUN_DISABLE;
        cis = 0;
    }

    // 时间基准计数
    It_Num++;
    It_Loop++;
}