#include "system/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "update.h"
#include "TPH/Au_Adc.h"
#include "app_power_manage.h"


/**
  * @brief   ADC去极值滤波
  * @param   channel: ADC通道
  * @param   sample_count: 采样次数(建议3~10)
  * @return  滤波后的ADC值
  */
u32 adc_filter_remove_extremes(u32 channel, u8 sample_count, u8 isvoltage)
{
    if(sample_count < 3) sample_count = 3;  // 至少3次采样
    
    u32 samples[sample_count];
    u32 sum = 0;
    u32 min_val, max_val;
    
    // 采集多个样本
    for(int i = 0; i < sample_count; i++) {
        if(!isvoltage)
            samples[i] = adc_get_value(channel); // 获取ADC原始值
        else
        samples[i] = adc_get_voltage(channel); // 获取电压值
        sum += samples[i];
    }
    
    // 找出最大值和最小值
    min_val = max_val = samples[0];
    for(int i = 1; i < sample_count; i++) {
        if(samples[i] < min_val) min_val = samples[i];
        if(samples[i] > max_val) max_val = samples[i];
    }
    
    // 去掉最大最小值后求平均
    return (sum - min_val - max_val) / (sample_count - 2);
}


/**
  * @brief   ADC 获取电压值
  * @return  滤波后的电压等级
  */
 u16 get_adc_level(u32 channel, u8 isvoltage)
 {
    // 指数平滑滤波参数: shift 为 N 则 alpha = 1/(2^N)
    // 范围: 2~6 (2: 快，6: 平滑)
    #ifndef VBAT_FILTER_SHIFT
    #define VBAT_FILTER_SHIFT    4      // 滤波平滑参数
    #endif

    #ifndef ALL_SAMPLE_COUNT  
    #define ALL_SAMPLE_COUNT    5      // 采样次数
    #endif

    // u32 raw = adc_get_voltage(AD_CH_VBAT);
    u32 raw = adc_filter_remove_extremes(channel, ALL_SAMPLE_COUNT, isvoltage);
#if (VBAT_FILTER_SHIFT > 0)
    static u32 vbat_filt = 0;
    if (vbat_filt == 0) {
        // 初始化滤波器状态
        vbat_filt = raw << VBAT_FILTER_SHIFT;
    } else {
        // vbat_filt = vbat_filt*(1 - 1/2^N) + raw*(1/2^N)
        vbat_filt += raw - (vbat_filt >> VBAT_FILTER_SHIFT);
    }
    raw = (u32)(vbat_filt >> VBAT_FILTER_SHIFT);
#endif

    return (raw * 4 / 10);
 }

/*
  * @brief  根据热敏电阻公式计算实际温度
  * @param  resistance: 热敏电阻阻值(欧姆)
  * @return 温度(摄氏度)	
  * @note   热敏电阻公式: Rt = Rp*Exp(B*(1/T1-1/T2)) -> T1 = 1/(log(Rt/Rp)/B+1/T2)
  * @note   T1和T2指的是卡尔文温度, K = 273.15 + 摄氏度
  * @note   Rt为热敏电阻在T1温度下的阻值
  * @note   最终对应的摄氏度T = T1 - 273.15, 同时+0.5的误差校正
 */
float calculate_temperature_from_resistance(float resistance)
{
    const float Rp = TEMP_NOMINAL_RESISTANCE;      // 25°C时的标称阻值
    const float T2 = 273.15f + 25.0f; // 参考温度(卡尔文)
    const float Bx = TEMP_B_COEFFICIENT;       // B值
    const float Ka = 273.15f;       // 卡尔文温度偏移
    const float error_correction = 0.5f; // 误差校正
    
    float temp = 0.0f;
    
    // 使用热敏电阻公式计算温度
    temp = 1.0f / (logf(resistance / Rp) / Bx + 1.0f / T2) - Ka + error_correction;
    
    return temp;
}


/*
  * @brief  将ADC值转换为热敏电阻阻值
  * @param  adc_value: ADC原始值
  * @param  vref: 参考电压(默认3.3V)
  * @param  series_resistor: 串联电阻阻值(默认10K)
  * @param  adc_resolution: ADC分辨率(默认4096)
  * @return 热敏电阻阻值(欧姆)	
  * @note   基于串联分压原理计算
 */
float convert_adc_to_resistance(uint32_t adc_value, float vref, float series_resistor, uint32_t adc_resolution)
{
    float voltage = 0.0f;
    float resistance = 0.0f;
    
    // ADC值转换为电压
    voltage = (float)adc_value * vref / (float)adc_resolution;
    
    // 电压转换为阻值 (串联分压公式: Rt = (Vout * R_series) / (Vref - Vout))
    if (voltage < vref) {
        resistance = (voltage * series_resistor) / (vref - voltage);
    } else {
        resistance = series_resistor * 100.0f; // 防止除零，返回一个大值
    }
    
    return resistance;
}


/*
  * @brief  获取温度
  * @param  none
  * @return 温度(摄氏度)	
  * @note   使用滤波后的ADC值进行计算
 */
float get_temperature(void)
{
    float temperature = 0.0f;
    float resistance = 0.0f;
    uint16_t filtered_adc = 0;
    
    // 使用你的通用ADC滤波函数获取温度通道的滤波值
    filtered_adc = get_adc_level(AD_CH_TEMP, Bit_RESET); 
    
    // 将ADC值转换为电阻值 (10K串联电阻，3.3V参考电压，12位ADC)
    resistance = convert_adc_to_resistance(filtered_adc, 3.3f, 10000.0f, 4096);
    
    // 根据电阻值计算温度
    temperature = calculate_temperature_from_resistance(resistance);
    
    return temperature;
}


/*
  * @brief  快速获取温度(简化版本)
  * @param  none
  * @return 温度(摄氏度)	
  * @note   适用于对实时性要求高的场景
 */
float get_temperature_quick(void)
{
    // 直接使用滤波后的ADC值计算温度
    uint16_t filtered_adc = get_adc_level(AD_CH_TEMP, Bit_RESET);
    float voltage = (float)filtered_adc * 3.3f / 4096.0f;
    float resistance = (voltage * 10000.0f) / (3.3f - voltage);
    
    return calculate_temperature_from_resistance(resistance);
}