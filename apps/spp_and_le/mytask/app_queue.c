#include "mytask/app_queue.h"

// 定义消息队列
static struct ble_data_queue {
    uint8_t data[256];
    uint16_t length;
    uint16_t handle;
} ble_queue;

// 创建消息队列或环形缓冲区
static struct ble_data_item {
    uint8_t data[256];
    uint16_t length;
    uint16_t handle;
} ble_data_queue[BLE_DATA_QUEUE_SIZE];

static int queue_write_index = 0;
static int queue_read_index = 0;
static OS_MUTEX ble_queue_mutex;

// 初始化队列
void ble_data_queue_init(void) {
    os_mutex_create(&ble_queue_mutex);
}

// 向队列添加数据
void ble_data_enqueue(uint8_t *data, uint16_t length, uint16_t handle) {
    os_mutex_pend(&ble_queue_mutex, 0);
    
    if (((queue_write_index + 1) % BLE_DATA_QUEUE_SIZE) != queue_read_index) {
        memcpy(ble_data_queue[queue_write_index].data, data, length);
        ble_data_queue[queue_write_index].length = length;
        ble_data_queue[queue_write_index].handle = handle;
        queue_write_index = (queue_write_index + 1) % BLE_DATA_QUEUE_SIZE;
    }
    
    os_mutex_post(&ble_queue_mutex);
}

// 从队列取出数据
int ble_data_dequeue(uint8_t *data, uint16_t *length, uint16_t *handle) {
    int result = 0;
    
    os_mutex_pend(&ble_queue_mutex, 0);
    
    if (queue_read_index != queue_write_index) {
        *length = ble_data_queue[queue_read_index].length;
        *handle = ble_data_queue[queue_read_index].handle;
        memcpy(data, ble_data_queue[queue_read_index].data, *length);
        queue_read_index = (queue_read_index + 1) % BLE_DATA_QUEUE_SIZE;
        result = 1;
    }
    
    os_mutex_post(&ble_queue_mutex);
    return result;
}