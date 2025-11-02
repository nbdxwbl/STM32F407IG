#include "buzzer.h"
#include "stm32f4xx_hal.h"

// 微秒级忙等待延时（基于指令空转），不依赖 SysTick
static void buzzer_delay_us(uint32_t us)
{
    // 经验系数：每次循环大约消耗 ~6 个 CPU 周期（取决于编译优化）
    // 与示例一致：cycles = us * (SystemCoreClock / 6_000_000)
    uint32_t cycles = us * (SystemCoreClock / 6000000UL);
    while (cycles--) {
        __NOP();
    }
}

void buzzer_beep(uint16_t frequency, uint16_t duration_ms)
{
    if (frequency == 0) {
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
        HAL_Delay(duration_ms);
        return;
    }

    // 半周期长度（微秒）
    uint32_t half_period_us = 500000UL / frequency;
    // 周期数 = 频率 * 时间（秒）
    uint32_t cycles = ((uint32_t)frequency * duration_ms) / 1000UL;

    for (uint32_t i = 0; i < cycles; ++i)
    {
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
        buzzer_delay_us(half_period_us);
        HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
        buzzer_delay_us(half_period_us);
    }
}
