#include "my_iic.h"
#include "delay_us.h"   

void CT_Delay(void)
{
	delay_us(2);
} 

void CT_IIC_Init(void)
{     
  GPIO_InitTypeDef  GPIO_InitStruct; 

		// 确保 us 级延时模块已按当前主频初始化，避免在 FreeRTOS 下 fac_us 为 0 导致无延时
		// HAL_RCC_GetHCLKFreq 以 Hz 返回，转为 MHz 供 delay_init 使用
		uint32_t mhz = HAL_RCC_GetHCLKFreq() / 1000000U;
		if (mhz == 0U || mhz > 255U) {
				mhz = 168U; // 兜底使用常见的 168MHz（STM32F407 默认最高主频）
		}
		delay_init((uint8_t)mhz);
	
	__HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_SET);  
	
	GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET); 
}

void CT_SDA_IN(void)  
{
	GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
}

void CT_SDA_OUT(void) 
{
	GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
}

void CT_IIC_Start(void)
{
	CT_SDA_OUT();     
	CT_IIC_SDA_1;     
	CT_IIC_SCL_1;
	delay_us(5);
 	CT_IIC_SDA_0; 
	delay_us(5);
	CT_IIC_SCL_0;
}

void CT_IIC_Stop(void)
{
	CT_SDA_OUT();
	CT_IIC_SCL_1;
	CT_IIC_SDA_0;
	delay_us(5);
	CT_IIC_SDA_1;  
	delay_us(5);
	CT_IIC_SCL_0;
}

uint8_t CT_IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	CT_SDA_IN();      
	CT_IIC_SDA_1;   delay_us(2);
	CT_IIC_SCL_1;   delay_us(2);
	while(CT_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			CT_IIC_Stop();
			return 1;
		} 
		CT_Delay();
	}
	CT_IIC_SCL_0;   
	return 0;  
} 

void CT_IIC_Ack(void)
{
	CT_IIC_SCL_0;
	CT_SDA_OUT();
	CT_Delay();
	CT_IIC_SDA_0;
	CT_Delay();CT_Delay();
	CT_IIC_SCL_1;
	CT_Delay();CT_Delay();
	CT_IIC_SCL_0;
}

void CT_IIC_NAck(void)    
{
	CT_IIC_SCL_0;
	CT_SDA_OUT();
	CT_Delay();
	CT_IIC_SDA_1;
	CT_Delay();
	CT_IIC_SCL_1;
	CT_Delay();
	CT_IIC_SCL_0;
}

void CT_IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	CT_SDA_OUT();     
    CT_IIC_SCL_0;
	CT_Delay();
	for(t=0;t<8;t++)
    {              
    	if(((txd&0x80)>>7)==1)CT_IIC_SDA_1;
			else CT_IIC_SDA_0;
        txd<<=1;   
			delay_us(5);      
		CT_IIC_SCL_1; 
			delay_us(5);
		CT_IIC_SCL_0;  
			delay_us(5);
    }  
}      

uint8_t CT_IIC_Read_Byte(unsigned char ack)
{
	uint8_t i,receive=0;
  	CT_SDA_IN();
	for(i=0;i<8;i++ )
	{ 
		CT_IIC_SCL_0;          
		delay_us(4);
		CT_IIC_SCL_1;   
		receive<<=1;
		if(CT_READ_SDA)receive++;  
		delay_us(4);  
	}   
	if (!ack)CT_IIC_NAck();
	else CT_IIC_Ack();   
 	return receive;
}
