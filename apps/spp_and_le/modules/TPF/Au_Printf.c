#include "system/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "update.h"
#include "TPH/Au_Printf.h"
#include "TPH/Au_Motor.h"
#include "TPH/Au_Timer.h"

#define LOW               0x0
#define HIGH              0x1

float addTime[6] = {0};
//点数-增加时间系数
#define kAddTime 0.001

//根据打印头实际打印效果修改打印时间偏移值
#define STB1_ADDTIME 0
#define STB2_ADDTIME 0
#define STB3_ADDTIME 0
#define STB4_ADDTIME 0
#define STB5_ADDTIME 0
#define STB6_ADDTIME 0

// 日志系统定义
#define LOG_TAG_CONST       APP
#define LOG_TAG             "[AU_PRINTF]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
#include "debug.h"

//热密度
uint8_t Heat_Density = 64;

/*
  * @brief  引脚写入电平
  * @param  pin:引脚, pinState:引脚状态
  * @return none	
  * @note   none
 */
static void Digital_Write(int pin, int pinState)
{
    switch (pin) {
        case PIN_STB1:
            gpio_direction_output(STB1_Pin, pinState);
            break;
        case PIN_STB2:
            gpio_direction_output(STB1_Pin, pinState);
            break;
        case PIN_STB3:
            gpio_direction_output(PIN_STB3, pinState);
            break;
        case PIN_STB4:
            gpio_direction_output(PIN_STB4, pinState);
            break;
        case PIN_STB5:
            gpio_direction_output(PIN_STB5, pinState);
            break;
        case PIN_STB6:
            gpio_direction_output(PIN_STB6, pinState);
            break;
        case PIN_LAT:
            gpio_direction_output(GPIO_Port_TPH_LAT, pinState);
            break;
        default:
            break;
    }
}

/*
  * @brief  VHEN引脚写入电平
  * @param  pin, pinState
  * @return none	
  * @note   none
 */
static void Digital_Write_Vhen(int pin,int PinState){
    PRINT_DEBUG("START VHEN");
	gpio_direction_output(PIN_VHEN,PinState);
}

/*
  * @brief  失能所有通道
  * @param  none
  * @return none	
  * @note   none
 */
static void Set_Stb_Unable(void)
{
    Digital_Write(PIN_STB1, LOW);
    Digital_Write(PIN_STB2, LOW);
    Digital_Write(PIN_STB3, LOW);
    Digital_Write(PIN_STB4, LOW);
    Digital_Write(PIN_STB5, LOW);
    Digital_Write(PIN_STB6, LOW);
}

/*
  * @brief  打印前初始化
  * @param  none
  * @return none	
  * @note   none
 */
static void Init_Printing(void)
{
    PRINT_DEBUG("Init Printing!");
    //开启打印超时监听
    Open_Printer_Timeout_Timer();
    //STB, LAT, VHEN ON
    Set_Stb_Unable();
    Digital_Write(PIN_LAT, HIGH);
    Digital_Write_Vhen(PIN_VHEN, HIGH);
}

/*
  * @brief  打印后停止
  * @param  none
  * @return none	
  * @note   none
 */
static void Stop_Printing(void)
{
    PRINT_DEBUG("Stop Printing!");
    //关闭打印超时监听
    Close_Printer_Timeout_Timer();
    //STB, LAT, VHEN OFF
    Digital_Write_Vhen(PIN_VHEN, LOW);
    Set_Stb_Unable(); /* 失能通道 */
    Digital_Write(PIN_LAT, HIGH);
}

/*
  * @brief  设置打印热密度
  * @param  Density:热密度
  * @return none	
  * @note   none
 */
void Set_Heat_Density(uint8_t Density)
{
    PRINT_DEBUG("打印密度设置%d\n", Density);
    Heat_Density = Density;
}

/*
  * @brief  清除增加时间数组
  * @param  none
  * @return none	
  * @note   none
 */
void Clear_AddTime(void)
{
    memset(addTime, 0, sizeof(addTime));
}


/*
  * @brief  发送一行数据
  * @param  data:数据
  * @return none	
  * @note   根据数据动态调整打印时间
 */
static void Send_ALine_Data(uint8_t* data)
{
    PRINT_DEBUG("Start send a line Data!"); 
    float tmpAddTime = 0;
    Clear_AddTime();
    
    for (uint8_t i = 0; i < 6; ++i) {
        for (uint8_t j = 0; j<8; ++j) {
            addTime[i] += data[i*8+j];
        }
        tmpAddTime = addTime[i] * addTime[i];
        addTime[i] = kAddTime * tmpAddTime;
    }
    
    // Spi_Command(data, TPH_DI_LEN);
    Digital_Write(PIN_LAT, LOW);
    us_delay(LAT_TIME);
    Digital_Write(PIN_LAT, HIGH);
}


/*
  * @brief  通道打印运行
  * @param  当前通道号
  * @return none	
  * @note   none
 */
static void Run_STB(uint8_t Now_STB_num)
{
    PRINT_DEBUG("Start Run STB!");
    switch (Now_STB_num) {
        case 0:
            Digital_Write(PIN_STB1, 1);
            us_delay((PRINT_TIME + addTime[0] + STB1_ADDTIME) * ((double)Heat_Density / 100));
            Digital_Write(PIN_STB1, 0);
            us_delay(PRINT_END_TIME);
            break;
        case 1:
            Digital_Write(PIN_STB2, 1);
            us_delay((PRINT_TIME + addTime[1] + STB2_ADDTIME) * ((double)Heat_Density / 100));
            Digital_Write(PIN_STB2, 0);
            us_delay(PRINT_END_TIME);
            break;
        case 2:
            Digital_Write(PIN_STB3, 1);
            us_delay((PRINT_TIME + addTime[2] + STB3_ADDTIME) * ((double)Heat_Density / 100));
            Digital_Write(PIN_STB3, 0);
            us_delay(PRINT_END_TIME);
            break;
        case 3:
            Digital_Write(PIN_STB4, 1);
            us_delay((PRINT_TIME + addTime[3] + STB4_ADDTIME) * ((double)Heat_Density / 100));
            Digital_Write(PIN_STB4, 0);
            us_delay(PRINT_END_TIME);
            break;
        case 4:
            Digital_Write(PIN_STB5, 1);
            us_delay((PRINT_TIME + addTime[4] + STB5_ADDTIME) * ((double)Heat_Density / 100));
            Digital_Write(PIN_STB5, 0);
            us_delay(PRINT_END_TIME);
            break;
        case 5:
            Digital_Write(PIN_STB6, 1);
            us_delay((PRINT_TIME + addTime[5] + STB6_ADDTIME) * ((double)Heat_Density / 100));
            Digital_Write(PIN_STB6, 0);
            us_delay(PRINT_END_TIME);
            break;
        default:
            break;
    }
}

/*
  * @brief  移动电机&开始打印
  * @param  needstop, STBnum
  * @return none	
  * @note   none
 */
bool Move_And_Start_STB(bool need_stop, uint8_t STBnum)
{
    if (need_stop == true) {
        PRINT_DEBUG("stop printing!");
        Motor_Stop();
        Stop_Printing();
        return true;
    }
    
    //4Steps a line
    Motor_Run();
    if (STBnum == ALL_STB_NUM) {
        //所有通道打印
        for (uint8_t index = 0; index<6; index++) {
            Run_STB(index);
            
            //在通道加热的同时将电机运行信号插入
            if (index == 1 || index == 3 || index == 5) {
                Motor_Run();
            }
        }
        
    }
    else {
        //单通道打印
        Run_STB(STBnum);
        Motor_Run_Steps(3);
    }
    
    return false;
}

/*
  * @brief  队列缓冲区打印
  * @param  none 
  * @return none	
  * @note   none
 */
void Start_Printing_By_QueueBuffer(void)
{
    uint8_t * pData = NULL;
    uint32_t PrinterCount = 0;
    Init_Printing();
    
    while (1) {
        if (Get_ble_rx_leftline() > 0) {
            //从缓冲区读取一行数据
            pData = Read_To_PrintBuffer();
            if (pData != NULL) {
                PrinterCount++;
                Send_ALine_Data(pData);
                if (Move_And_Start_STB(false, ALL_STB_NUM)) {
                    break;
                }
            }
        }
        else {
            //停止打印
            if (Move_And_Start_STB(true, ALL_STB_NUM)) {
                break;
            }
        }
        
        if (Get_Printer_Timeout_Status()) {
            break;
        }
    }
    
    Motor_Run_Steps(100);
    Motor_Stop();
    Clean_Blepack_Count();
    PRINT_DEBUG("Start_Printing_By_QueueBuffer: PrintingFinish, Recv: %d", PrinterCount);
}
  
/*
  * @brief  单通道打印
  * @param  STBnum:通道号, Data:数据
  * @return none	
  * @note   none
 */
void Start_Printing_By_OneSTB(uint8_t STBnum, uint8_t* Data, uint32_t Len)
{
    PRINT_DEBUG("Start Printing By One STB!");
    if (TPH_DI_LEN == 0 || Data == NULL || Len == 0) {
        log_info("invalid print params: TPH_DI_LEN=%d, Len=%u", TPH_DI_LEN, Len);
        return;
    }
    uint32_t offset = 0;
    uint8_t* ptr = Data;
    bool need_stop = false;
    Init_Printing();
    
    while (1) {
        PRINT_DEBUG("printer %d", offset);
        if (Len > offset) {
            //发送一行数据 48Byte
            Send_ALine_Data(ptr);
            offset += TPH_DI_LEN;
            ptr += TPH_DI_LEN;
        }
        else {
            need_stop = true;
        }
        
        if (Move_And_Start_STB(need_stop, STBnum)) {
            break;
        }
        if (Get_Printer_Timeout_Status()) {
            break;
        }
    }
    Motor_Run_Steps(40);
    Motor_Stop();
}

/*
  * @brief  设置TESTSTB数据
  * @param  Print_Data:传入数组
  * @return none	
  * @note   none
 */
static void Set_Debug_Data(uint8_t* Print_Data)
{
    for (uint32_t len = 0; len < 48*5; len++){
        Print_Data[len] = 0x55;
    }
    PRINT_DEBUG("Finish Set Debug Data");
}


/*
  * @brief  测试打印模块
  * @param  none
  * @return none	
  * @note   none
 */
void TestSTB(void)
{
    uint8_t Print_Data[48*6];
    //每行48个字节，48*8=384个像素点，打印5行
    uint32_t Print_Len;
    Print_Len = 48 * 5;
    PRINT_DEBUG("Start TestSTB, Sequence:123456!");
    
    Set_Debug_Data(Print_Data);
    Start_Printing_By_OneSTB(0, Print_Data, Print_Len);
    Set_Debug_Data(Print_Data);
    Start_Printing_By_OneSTB(1, Print_Data, Print_Len);
    Set_Debug_Data(Print_Data);
    Start_Printing_By_OneSTB(2, Print_Data, Print_Len);
    Set_Debug_Data(Print_Data);
    Start_Printing_By_OneSTB(3, Print_Data, Print_Len);
    Set_Debug_Data(Print_Data);
    Start_Printing_By_OneSTB(4, Print_Data, Print_Len);
    Set_Debug_Data(Print_Data);
    Start_Printing_By_OneSTB(5, Print_Data, Print_Len);
    
    PRINT_DEBUG("Finish TestSTB~");
}

/*
  * @brief  打印模块初始化
  * @param  none
  * @return none	
  * @note   none
 */
void Init_Printer(void)
{
    Init_Motor();
    Set_Stb_Unable();
    Digital_Write_Vhen(PIN_VHEN, RESET);
    // Init_Spi();
}