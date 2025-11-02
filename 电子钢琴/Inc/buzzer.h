#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "stm32f4xx_hal.h"
#include "gpio.h"
#include <stdint.h>

#ifndef BUZZER_PORT
#define BUZZER_PORT GPIOG
#endif

#ifndef BUZZER_PIN
#define BUZZER_PIN GPIO_PIN_6
#endif

// 播放指定频率的方波，持续 duration_ms 毫秒。
// 注意：该实现内部使用忙等待的微秒延时，仅用于短片段播放，
// 在 RTOS 下建议配合低优先级任务并在外层分片调用。
void buzzer_beep(uint16_t frequency, uint16_t duration_ms);

#endif // __BUZZER_H__
