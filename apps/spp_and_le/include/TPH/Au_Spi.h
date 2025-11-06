#ifndef __AU_SPI_H__
#define __AU_SPI_H__

#include "Au_Config.h"
#include "Au_Printf.h"

#define TPH_HLAT(Temp)		gpio_direction_output(GPIO_Port_TPH_LAT, Temp)
#define TPH_SCLK(Temp)		gpio_direction_output(GPIO_Port_TPH_CLK, Temp)
#define TPH_HDI(Temp)		gpio_direction_output(GPIO_Port_TPH_DI, Temp)	

void Spi_Command(u8 data_buffer);
void TPH_WR_Byte(u8 dat);

#endif
