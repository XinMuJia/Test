#include "system/includes.h"
#include "app_config.h"
#include "mytask/app_time.h"
#include "rtc/virtual_rtc.h"
#include "asm/rtc.h"
#include "ioctl.h"
#include "generic/printf.h"




extern struct sys_time rtc_read_test;

#if TCFG_USE_VIRTUAL_RTC
// 使用虚拟RTC获取时间
void get_current_time(struct sys_time *curr_time)
{
    if (curr_time) {
        // read_sys_time(curr_time);
        // curr_time = __TIME__;
        
        vir_rtc_get_time(curr_time);
    }
}

// 设置系统时间
void set_system_time(struct sys_time *new_time)
{
    if (new_time) {
        // write_sys_time(new_time);
        vir_rtc_set_time(new_time);
    }
}


void app_time_init(void)
{
    struct sys_time tmp_time;
    memset((u8 *)&tmp_time, 0, sizeof(tmp_time));

    // 初始化虚拟RTC
    vir_rtc_init();
    // 启动RTC
    // rtc_init(&rtc_data); 
}
#else
#if TCFG_RTC_ALARM_ENABLE
// 使用硬件RTC获取时间
void get_current_time(struct sys_time *curr_time)
{
    if (curr_time) {
        read_sys_time(curr_time);
        // dev_ioctl(__this->dev_handle, IOCTL_GET_SYS_TIME, curr_time);
        // dev_ioctl(  , IOCTL_GET_SYS_TIME, curr_time);
        // rtc_ioctl(IOCTL_GET_SYS_TIME, curr_time); //读时钟

    }

}

// 设置系统时间
void set_system_time(struct sys_time *new_time)
{
    if (new_time) {
        write_sys_time(new_time);
        
    }
}
#else
// 如果都没有启用RTC，使用系统运行时间模拟
// 软件RTC实现
static struct sys_time software_rtc = {0};
static u32 last_sys_time_ms = 0;
static u8 software_rtc_initialized = 0;
/* 判断是否闰年 */
static int is_leap_year(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/* 返回指定年月的天数 */
static int days_in_month(int month, int year)
{
    static const int mdays[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if (month == 2) {
        return mdays[1] + (is_leap_year(year) ? 1 : 0);
    } else if (month >= 1 && month <= 12) {
        return mdays[month - 1];
    }
    return 31;
}

/* 归一化时间字段，使秒<60，分<60，时<24，日合法（根据月份和闰年） */
static void normalize_time(struct sys_time *t)
{
    if (!t) {
        return;
    }

    /* 秒进位到分 */
    if (t->sec >= 60) {
        t->min += t->sec / 60;
        t->sec %= 60;
    } else if ((int)t->sec < 0) {
        /* 不应出现的情况，截断为0 */
        t->sec = 0;
    }

    /* 分进位到小时 */
    if (t->min >= 60) {
        t->hour += t->min / 60;
        t->min %= 60;
    } else if ((int)t->min < 0) {
        t->min = 0;
    }

    /* 小时进位到日 */
    if (t->hour >= 24) {
        t->day += t->hour / 24;
        t->hour %= 24;
    } else if ((int)t->hour < 0) {
        t->hour = 0;
    }

    /* 日进位到月/年 */
    while (1) {
        int dim = days_in_month(t->month, t->year);
        if (t->day <= dim) {
            break;
        }
        t->day -= dim;
        t->month++;
        if (t->month > 12) {
            t->month = 1;
            t->year++;
        }
    }
}

/* 向 系统时间 添加秒（可为较大值） */
static void add_seconds_to_time(struct sys_time *t, u32 seconds)
{
    if (!t) {
        return;
    }

    /* 添加整秒到秒/分/时字段 */
    t->sec += seconds % 60;
    t->min += (seconds / 60) % 60;
    t->hour += (seconds / 3600) % 24;
    /* 要添加的天数 */
    u32 days = seconds / 86400;
    t->day += days;

    /* 归一化所有字段（秒/分/时/日 -> 月/年） */
    normalize_time(t);
}

/* 由定时器周期调用，每次将软件 RTC 前进 1 秒 */
void update_software_rtc(void *priv)
{
    if (!software_rtc_initialized) {
        return;
    }

    /* 增加一秒 */
    software_rtc.sec++;
    normalize_time(&software_rtc);

    /* 更新最后刻度时间 */
    last_sys_time_ms = sys_timer_get_ms();
}

// 解析编译时间字符串
static void parse_compile_time(struct sys_time *time) {
    // __DATE__ 格式示例: "Oct 23 2025"
    // __TIME__ 格式示例: "14:25:36"
    
    const char *date_str = __DATE__;
    const char *time_str = __TIME__;
    
    // 月份映射表
    static const char months[][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    
    char month_str[4];
    int year, day;
    
    // 解析日期
    sscanf(date_str, "%3s %d %d", month_str, &day, &year);
    
    // 查找月份
    int month = 1;
    for (int i = 0; i < 12; i++) {
        if (strncmp(month_str, months[i], 3) == 0) {
            month = i + 1;
            break;
        }
    }
    
    // 解析时间
    int hour, min, sec;
    sscanf(time_str, "%d:%d:%d", &hour, &min, &sec);
    
    // 赋值到结构体
    time->year = year;
    time->month = month;
    time->day = day;
    time->hour = hour;
    time->min = min;
    time->sec = sec;
}

/* 初始化软件 RTC 并启动周期更新器 */
void init_software_rtc(void)
{
    if (software_rtc_initialized) {
        return;
    }

    // 使用编译时间作为默认时间
    // parse_compile_time(&software_rtc);

    // /* 边界检查并修正 */
    // if (software_rtc.year < 2020 || software_rtc.year > 2030) {
    //     software_rtc.year = 2025;
    // }

    // if (software_rtc.month < 1) {
    //     software_rtc.month = 1;
    // } else if (software_rtc.month > 12) {
    //     software_rtc.month = 12;
    // }

    // {
    //     int dim = days_in_month(software_rtc.month, software_rtc.year);
    //     if (software_rtc.day < 1) {
    //         software_rtc.day = 1;
    //     } else if (software_rtc.day > dim) {
    //         software_rtc.day = dim;
    //     }
    // }

    // if (software_rtc.hour > 23)  software_rtc.hour = 0;
    // if (software_rtc.min  > 59)  software_rtc.min  = 0;
    // if (software_rtc.sec  > 59)  software_rtc.sec  = 0;

    software_rtc.year = 2025;
    software_rtc.month = 1;
    software_rtc.day = 1;
    software_rtc.hour = 12;
    software_rtc.min = 0;
    software_rtc.sec = 0;

    // set_system_time(&software_rtc);
    last_sys_time_ms = sys_timer_get_ms();
    software_rtc_initialized = 1;

    /* 创建 1 秒周期定时器以保持软件 RTC 运行 */
    // sys_timer_add(NULL, update_software_rtc, 1000);
    sys_hi_timer_add(NULL, update_software_rtc, 1000);
}

/* 从软件 RTC 获取当前时间（计算自上次基准以来的增量） */
void get_current_time(struct sys_time *curr_time)
{
    if (!curr_time) {
        return;
    }

    if (!software_rtc_initialized) {
        init_software_rtc();
    }

    /* 复制基准时间 */
    struct sys_time tmp = software_rtc;

    /* 计算自上次快照以来的毫秒差并添加秒 */
    u32 now_ms = sys_timer_get_ms();
    u32 delta_ms = (now_ms >= last_sys_time_ms) ? (now_ms - last_sys_time_ms) : 0;
    if (delta_ms >= 1000) {
        add_seconds_to_time(&tmp, delta_ms / 1000);
    }

    *curr_time = tmp;
}

/* 将软件 RTC 设置为提供的时间 */
void set_system_time(struct sys_time *new_time)
{
    if (!new_time) {
        return;
    }

    if (!software_rtc_initialized) {
        init_software_rtc();
    }

    /* 复制并归一化 */
    software_rtc = *new_time;
    normalize_time(&software_rtc);
    last_sys_time_ms = sys_timer_get_ms();
}
#endif
#endif