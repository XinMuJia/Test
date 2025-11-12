#ifndef __APP_QUEUE_H__
#define __APP_QUEUE_H__

#include "typedef.h"
#include "TPH/Au_Config.h"

// 队列大小定义
#define BLE_DATA_QUEUE_SIZE 10

// 函数声明
void ble_data_queue_init(void);
void ble_data_enqueue(uint8_t *data, uint16_t length, uint16_t handle);
int ble_data_dequeue(uint8_t *data, uint16_t *length, uint16_t *handle);

#endif  