#include "lcd_touch.h"
#include "lcd.h"
#include "gt9147.h"
#include <stdio.h>


tp_dev_t tp_dev;

// 触摸初始化
uint8_t TP_Init(void)
{
  GT9147_Init();
  tp_dev.touchtype|=0X80;   // 触摸有效
  tp_dev.touchtype|=lcddev.dir&0X01; // 与 LCD 方向一致
  return 0;
}

uint8_t Touch_Scan(uint8_t mode)
{
	return GT9147_Scan(mode);
}
