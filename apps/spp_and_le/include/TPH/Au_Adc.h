#ifndef     __AU_ADC_H__
#define     __AU_ADC_H__
#include "typedef.h"

// ADC通道定义
#define AD_CH_TEMP             AD_CH_PB10 // 热敏电阻接在PB10通道

// 温度传感器参数配置
#define TEMP_SERIES_RESISTOR   10000.0f    // 串联电阻 10K
#define TEMP_NOMINAL_RESISTANCE 30000.0f   // 25°C标称阻值 30K
#define TEMP_NOMINAL_TEMP      25.0f       // 标称温度 25°C
#define TEMP_B_COEFFICIENT     3950.0f     // B值 3950
#define TEMP_VOLTAGE_REF       3.3f        // 参考电压 3.3V
#define TEMP_ADC_RESOLUTION    4096        // ADC分辨率 12位

/*  函数声明  */
u32 adc_filter_remove_extremes(u32 channel, u8 sample_count);
u16 get_adc_level(u32 channel);
float calculate_temperature_from_resistance(float resistance);
float convert_adc_to_resistance(uint32_t adc_value, float vref, float series_resistor, uint32_t adc_resolution);
float get_temperature(void);
float get_temperature_quick(void);

#endif  /*__AU_ADC_H__*/