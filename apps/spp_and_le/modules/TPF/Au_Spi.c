#include "TPH/Au_Spi.h"

/*
  * @brief  模拟SPI发送数据
  * @param  data_buffer:打印数据, data_len:数据长度
  * @return none	
  * @note   none
 */
void TPH_WR_Byte(u8 dat)
{
		if(dat&0x88)							//dat高位写入
		{
			TPH_HDI(Bit_SET);
    }
		
		else
		{
			TPH_HDI(Bit_RESET);
    }
		
		TPH_SCLK(Bit_SET);				//时钟信号跳转
		TPH_SCLK(Bit_RESET);			
}

void Spi_Command(u8 data_buffer)
{
    u8 i;
    u8 DATA1;
    
    for(i = 0; i < 8; i++) {
        DATA1 = 0x00;
        if(data_buffer & 0x80) {
            DATA1 = 0x88;
        }
        TPH_HDI(DATA1 ? Bit_SET : Bit_RESET);
        
        TPH_SCLK(Bit_SET);
        TPH_SCLK(Bit_RESET);
        
        data_buffer <<= 1;
    }
}
