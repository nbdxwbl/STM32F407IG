#ifndef __MYCT_IIC_H
#define __MYCT_IIC_H

#include "main.h"

// Bit-bang I2C pins (SCL: PG7, SDA: PI3)
#define CT_IIC_SCL_1     HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_SET)
#define CT_IIC_SCL_0     HAL_GPIO_WritePin(GPIOG, GPIO_PIN_7, GPIO_PIN_RESET)
#define CT_IIC_SDA_1     HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_SET)
#define CT_IIC_SDA_0     HAL_GPIO_WritePin(GPIOI, GPIO_PIN_3, GPIO_PIN_RESET)

#define CT_READ_SDA      HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_3)

void CT_IIC_Init(void);
void CT_IIC_Start(void);
void CT_IIC_Stop(void);
void CT_IIC_Send_Byte(uint8_t txd);
uint8_t CT_IIC_Read_Byte(unsigned char ack);
uint8_t CT_IIC_Wait_Ack(void);
void CT_IIC_Ack(void);
void CT_IIC_NAck(void);

void CT_SDA_IN(void);
void CT_SDA_OUT(void);

#endif
