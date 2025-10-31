#include "TPH/Au_Spi.h"

/*
  * @brief  对HAL库SPI的一层封装
  * @param  data_buffer:打印数据, data_len:数据长度
  * @return none	
  * @note   none
 */
void Spi_Command(u8 data_buffer)
{
    if(data_buffer&0x88)							//dat高位写入
		{
			//printf("1");
			TPH_HDI(Bit_SET);
	//		Delay1ns(2);
    }
		
		else
		{
			//printf("0");
			TPH_HDI(Bit_RESET);
		//	Delay1ns(2);
    }
		
		TPH_SCLK(Bit_SET);;				//时钟信号跳转
	//	Delay1ns(2);
		TPH_SCLK(Bit_RESET);	
}
