/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "tim.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "lcd.h"
#include "lcd_touch.h"
#include "piano.h"
#include "buzzer.h"
#include "gpio.h"

#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ENABLE_PRINT_TASK 0

#define KEY_MARGIN_LEFT    56
#define KEY_MARGIN_RIGHT   56
#define KEY_MARGIN_TOP     80
#define KEY_MARGIN_BOTTOM  80
#define WHITE_KEY_W        86
#define WHITE_KEY_H        320
#define BLACK_KEY_W        54
#define BLACK_KEY_H        244

// 音符频率（Hz）
#define NOTE_C4   262
#define NOTE_CS4  277
#define NOTE_D4   294
#define NOTE_DS4  311
#define NOTE_E4   330
#define NOTE_F4   349
#define NOTE_FS4  370
#define NOTE_G4   392
#define NOTE_GS4  415
#define NOTE_A4   440
#define NOTE_AS4  466
#define NOTE_B4   494
#define NOTE_C5   523

// 新增：舵机脉宽范围（微秒）
#define SERVO_MIN_US     600
#define SERVO_MAX_US     2400
#define SERVO_CENTER_US  1500
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

static volatile uint16_t g_current_freq = 0; // 当前发声频率，0 表示静音
// UI 初始化完成信号量：用于确保触摸任务在 LCD 方向/分辨率就绪后再初始化触摸芯片
static osSemaphoreId_t uiReadySem = NULL;
// UI 就绪标志：用于让多个任务感知 UI 已完成初始化（信号量只能被一个任务获取）
static volatile uint8_t g_ui_ready = 0;
// 音乐播放控制：使用线程标志触发（ISR 安全）与播放状态
static osThreadId_t musicTaskHandle = NULL;
static volatile uint8_t g_music_playing = 0;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void UITask(void *argument);
static void TouchTask(void *argument);
static void BuzzerTask(void *argument);
static void LEDTask(void *argument);
static void PrintTask(void *argument);
static uint16_t map_touch_to_freq(uint16_t x, uint16_t y);
static inline uint8_t in_range(int32_t v, int32_t a, int32_t b) { return (v >= a) && (v < b); }
static void ServoTask(void *argument);
static uint16_t map_freq_to_servo_pulse(uint16_t f);
static uint16_t map_freq_to_led_period(uint16_t f);
static void MusicTask(void *argument);
enum { MUSIC_FLAG_PLAY = 0x01 };
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  // 创建用于启动顺序协调的二值信号量（初始为0，UI完成后释放）
  uiReadySem = osSemaphoreNew(1, 0, NULL);
  // UI 任务：初始化 LCD 并显示钢琴图
  const osThreadAttr_t uiTask_attributes = {
    .name = "uiTask",
    .priority = (osPriority_t) osPriorityAboveNormal,
    .stack_size = 512
  };
  (void)osThreadNew(UITask, NULL, &uiTask_attributes);

  // 触摸扫描任务：读取坐标并更新频率
  const osThreadAttr_t touchTask_attributes = {
    .name = "touchTask",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 512
  };
  (void)osThreadNew(TouchTask, NULL, &touchTask_attributes);

  // 蜂鸣器任务：根据当前频率发声
  const osThreadAttr_t buzzerTask_attributes = {
    .name = "buzzerTask",
    .priority = (osPriority_t) osPriorityBelowNormal,
    .stack_size = 256
  };
  (void)osThreadNew(BuzzerTask, NULL, &buzzerTask_attributes);

  // LED 指示任务
  const osThreadAttr_t ledTask_attributes = {
    .name = "LED",
    .priority = (osPriority_t) osPriorityHigh,
    .stack_size = 128
  };
  (void)osThreadNew(LEDTask, NULL, &ledTask_attributes);

  // 新增：舵机任务（读取当前音高 -> 角度 -> PWM 脉宽）
  const osThreadAttr_t servoTask_attributes = {
    .name = "Servo",
    .priority = (osPriority_t) osPriorityNormal,
    .stack_size = 256
  };
  (void)osThreadNew(ServoTask, NULL, &servoTask_attributes);

  // 示例音乐任务：等待按键信号后播放一段旋律
  const osThreadAttr_t musicTask_attributes = {
    .name = "Music",
    .priority = (osPriority_t) osPriorityAboveNormal, // 高于触摸与蜂鸣器，便于稳定推进乐谱
    .stack_size = 512
  };
  musicTaskHandle = osThreadNew(MusicTask, NULL, &musicTask_attributes);

  // 串口打印任务
  #if ENABLE_PRINT_TASK
  const osThreadAttr_t printTask_attributes = {
    .name = "Print",
    .priority = (osPriority_t) osPriorityLow,
    .stack_size = 128
  };
  (void)osThreadNew(PrintTask, NULL, &printTask_attributes);
  #endif
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

// UI 任务：初始化延时与 LCD，并绘制钢琴整图
static void UITask(void *argument)
{
  // 延时与 LCD 初始化（与 FSMC 显示配合）
  // 注意：在 FreeRTOS 中不要调用会重配置 SysTick 的 delay_init，
  // 否则可能破坏内核节拍导致系统无响应。
  // delay_init(168);
  LCD_Init();
  if (uiReadySem) {
    osSemaphoreRelease(uiReadySem);
  }
  // 置 UI 就绪标志，并确保 LED 熄灭
  g_ui_ready = 1;
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
  LCD_Clear(BLACK);
  // 全屏显示钢琴图（piano.h 的前 6 字节包含 800x480 信息）
  LCD_DrawPicture(0, 0, 800, 480, (uint8_t*)gImage_piano);

  // 可选：提示文字
  LCD_ShowString(10, 10, 300, 24, 16, (uint8_t*)"Piano Ready");

  for(;;)
  {
    osDelay(1000);
  }
}

static uint16_t map_touch_to_freq(uint16_t x, uint16_t y)
{
  const int32_t key_top = KEY_MARGIN_TOP;
  const int32_t white_bottom = KEY_MARGIN_TOP + WHITE_KEY_H;   // 到 400
  const int32_t black_bottom = KEY_MARGIN_TOP + BLACK_KEY_H;   // 到 324

  // 黑键优先
  if (in_range((int32_t)y, key_top, black_bottom))
  {
    const uint8_t black_boundaries[5] = {1, 2, 4, 5, 6};
    for (uint8_t i = 0; i < 5; ++i)
    {
      int32_t b = black_boundaries[i];
      int32_t boundary_x = KEY_MARGIN_LEFT + b * WHITE_KEY_W;
      int32_t half_bw = BLACK_KEY_W / 2;
      if (in_range((int32_t)x, boundary_x - half_bw, boundary_x + half_bw))
      {
        switch (b)
        {
          case 1: return NOTE_CS4; // C#4
          case 2: return NOTE_DS4; // D#4
          case 4: return NOTE_FS4; // F#4
          case 5: return NOTE_GS4; // G#4
          case 6: return NOTE_AS4; // A#4
          default: break;
        }
      }
    }
  }

  // 白键区域
  if (in_range((int32_t)y, key_top, white_bottom))
  {
    int32_t x0 = KEY_MARGIN_LEFT;
    int32_t x1 = SSD_HOR_RESOLUTION - KEY_MARGIN_RIGHT; // 800 - 56 = 744
    if (in_range((int32_t)x, x0, x1))
    {
      uint8_t idx = (uint8_t)(((int32_t)x - x0) / WHITE_KEY_W); // 0..7
      switch (idx)
      {
        case 0: return NOTE_C4;
        case 1: return NOTE_D4;
        case 2: return NOTE_E4;
        case 3: return NOTE_F4;
        case 4: return NOTE_G4;
        case 5: return NOTE_A4;
        case 6: return NOTE_B4;
        case 7: return NOTE_C5;
        default: break;
      }
    }
  }
  return 0; // 未命中键区
}

// 触摸扫描任务：读取坐标并更新当前频率
static void TouchTask(void *argument)
{
  if (uiReadySem) {
    (void)osSemaphoreAcquire(uiReadySem, osWaitForever);
  }
  TP_Init();
  for(;;)
  {
    Touch_Scan(0); // 更新 tp_dev
    if (g_music_playing)
    {
      // 播放示例音乐期间，不接受触摸改频，避免打断旋律
    }
    else if (tp_dev.sta & TP_PRES_DOWN)
    {
      uint16_t x = tp_dev.x[0];
      uint16_t y = tp_dev.y[0];
      g_current_freq = map_touch_to_freq(x, y);
    }
    else
    {
      g_current_freq = 0; // 无触控静音
    }
    osDelay(15); // ~66Hz 扫描
  }
}

// 蜂鸣器任务：根据当前频率输出波形（PG6）
static void BuzzerTask(void *argument)
{
  for(;;)
  {
    uint16_t f = g_current_freq;
    if (f == 0)
    {
      HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
      osDelay(10);
    }
    else
    {
      // 播放一个很短的音频片段（20ms），随后让出 CPU
      buzzer_beep(f, 20);
      osDelay(1);
    }
  }
}

// LED 指示任务：轮流点亮/熄灭两个 LED（PC0 与 PB15）
static void LEDTask(void *argument)
{
  // 等待 UI 初始化完成后，先熄灭 LED
  while (!g_ui_ready) { osDelay(10); }
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);

  uint8_t led_on = 0;
  for(;;)
  {
    uint16_t f = g_current_freq;
    if (f == 0)
    {
      // 无按键：保持熄灭
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
      led_on = 0;
      osDelay(30);
      continue;
    }

    // 有按键：按音高映射后的节奏律动亮灭（双灯同闪）
    uint16_t period_ms = map_freq_to_led_period(f);
    uint16_t half = period_ms / 2;
    if (!led_on) {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
      led_on = 1;
      osDelay(half);
    } else {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
      led_on = 0;
      osDelay(half);
    }
  }
}

// 串口打印任务：周期性打印提示
static void PrintTask(void *argument)
{
  for(;;)
  {
    printf("Enjoy Music\n");
    osDelay(2000);
  }
}

// 将频率映射为舵机 PWM 脉宽（us）
static uint16_t map_freq_to_servo_pulse(uint16_t f)
{
  if (f == 0) {
    return SERVO_CENTER_US; // 无触控时居中
  }
  if (f < NOTE_C4) f = NOTE_C4;
  if (f > NOTE_C5) f = NOTE_C5;

  // 线性映射 C4..C5 -> 0..1 -> 600..2400us
  uint32_t num = (uint32_t)(f - NOTE_C4);
  uint32_t den = (uint32_t)(NOTE_C5 - NOTE_C4);
  uint32_t us  = SERVO_MIN_US + ((uint32_t)(SERVO_MAX_US - SERVO_MIN_US) * num) / den;

  if (us < SERVO_MIN_US) us = SERVO_MIN_US;
  if (us > SERVO_MAX_US) us = SERVO_MAX_US;
  return (uint16_t)us;
}

static void ServoTask(void *argument)
{
  // 启动 PWM（只需一次）
  HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);

  // 初始置中
  __HAL_TIM_SetCompare(&htim12, TIM_CHANNEL_1, SERVO_CENTER_US);

  uint16_t last_ccr = 0xFFFF;

  for(;;)
  {
    uint16_t f = g_current_freq;                       // 当前音高频率
    uint16_t ccr = map_freq_to_servo_pulse(f);         // 频率 -> 脉宽(us)

    // 只有变化时才更新，减少抖动
    if (ccr != last_ccr) {
      __HAL_TIM_SetCompare(&htim12, TIM_CHANNEL_1, ccr);
      last_ccr = ccr;
    }

    // 舵机典型周期 20ms，更新频率无需过高
    osDelay(20);
  }
}

// 示例音乐任务：等待按键触发，按乐谱设置当前频率
static void MusicTask(void *argument)
{
  typedef struct { uint16_t f; uint16_t ms; } Note;
  // 选取简单、悦耳的示例旋律（包含停顿0Hz）
  static const Note score[] = {
    {262, 300}, {294, 300}, {330, 300}, {349, 300}, {392, 400}, {0, 150},
    {392, 200}, {349, 200}, {330, 300}, {294, 300}, {262, 600}, {0, 300},
    // 段落2
    {330, 300}, {349, 300}, {392, 300}, {392, 300}, {349, 300}, {330, 300}, {294, 600}, {0, 300},
    // 结尾
    {262, 300}, {262, 300}, {262, 300}, {294, 300}, {294, 300}, {294, 300}, {262, 700}
  };
  const size_t score_len = sizeof(score)/sizeof(score[0]);

  for(;;)
  {
  // 等待按键中断发出的播放信号（线程标志，ISR 安全）
  (void)osThreadFlagsWait(MUSIC_FLAG_PLAY, osFlagsWaitAny, osWaitForever);

    // 若已在播放则忽略本次触发
    if (g_music_playing) { continue; }
    g_music_playing = 1;

    // 逐音符播放：通过共享的 g_current_freq 让蜂鸣器/LED/舵机联动
    for (size_t i = 0; i < score_len; ++i)
    {
      g_current_freq = score[i].f;    // 0 表示停顿
      osDelay(score[i].ms);
    }
    // 收尾：复位频率，清除播放标志
    g_current_freq = 0;
    g_music_playing = 0;
  }
}

// EXTI 按键回调：按下后触发音乐播放
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  // 参照默认使用 PA0（KEY/USER 按键）。如需更换请修改 gpio.c 中的引脚配置与此处判断。
  if (GPIO_Pin == GPIO_PIN_9)
  {
    // 防抖：简单判定 UI 已就绪；播放中则忽略
    if (!g_ui_ready) return;
    if (g_music_playing) return;
    // 设置线程标志启动音乐
    if (musicTaskHandle) {
      (void)osThreadFlagsSet(musicTaskHandle, MUSIC_FLAG_PLAY);
    }
  }
	else if (GPIO_Pin == GPIO_PIN_11)
  {
    // 防抖：简单判定 UI 已就绪；播放中则忽略
    if (!g_ui_ready) return;
    if (!g_music_playing) return;
    if (musicTaskHandle) {
      (void)osThreadFlagsSet(musicTaskHandle, MUSIC_FLAG_PLAY);
    }
  }
}

// 将频率映射为 LED 闪烁周期（ms），音高越高闪烁越快（可见范围内）
static uint16_t map_freq_to_led_period(uint16_t f)
{
  if (f == 0) return 0;
  if (f < NOTE_C4) f = NOTE_C4;
  if (f > NOTE_C5) f = NOTE_C5;

  const uint16_t max_ms = 600; // 低音：慢
  const uint16_t min_ms = 180; // 高音：快（但仍可见）
  uint32_t num = (uint32_t)(f - NOTE_C4);
  uint32_t den = (uint32_t)(NOTE_C5 - NOTE_C4);
  uint32_t period = max_ms - ((uint32_t)(max_ms - min_ms) * num) / den;
  if (period < min_ms) period = min_ms;
  if (period > max_ms) period = max_ms;
  return (uint16_t)period;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
