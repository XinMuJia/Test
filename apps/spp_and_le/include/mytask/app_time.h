#ifndef _APP_TIME_H_
#define _APP_TIME_H_

void get_current_time(struct sys_time *curr_time);
void set_system_time(struct sys_time *new_time);
void init_software_rtc(void);
// void app_time_init();
#endif /*_APP_TIME_H_*/