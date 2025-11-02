/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "cmsis_os.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
static  volatile int flag = 0 ;
static  volatile int alarm = 0 ;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
__IO uint16_t adcx[4]={0};
float temp0,temp1,temp2,temp3;
#define ZLG_READ_ADDRESS1         0x01 
#define ZLG_WRITE_ADDRESS1        0x17		//显示缓冲区首地址
#define ZLG_WRITE_ADDRESS2        0x16
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void LedD3(void const *argument);
void LedD2(void const *argument);
void stopAll(void const *argument);
void SensorTask(void const *argument);
void Peer(void const *argument);
void duoji(void const *argument);
void Love(void const *argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC3_Init();
  MX_I2C1_Init();
  MX_TIM12_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	HAL_ADC_Start_DMA(&hadc3,(uint32_t*)adcx,4);//开启ADC转换
	HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);
	HAL_GPIO_WritePin(GPIOB ,GPIO_PIN_15 ,GPIO_PIN_SET );
	HAL_GPIO_WritePin(GPIOC ,GPIO_PIN_0 ,GPIO_PIN_SET );
	HAL_GPIO_WritePin(GPIOF ,GPIO_PIN_10 ,GPIO_PIN_SET );
	HAL_GPIO_WritePin(GPIOH ,GPIO_PIN_15 ,GPIO_PIN_SET );
	osThreadDef (LedD3 ,LedD3 ,osPriorityNormal ,0,128);
	osThreadDef (LedD2 ,LedD2 ,osPriorityNormal ,0,128);
	osThreadDef (stopAll ,stopAll ,osPriorityNormal ,0,128);
	osThreadDef (SensorTask ,SensorTask ,osPriorityNormal ,0,128);
	osThreadDef (Peer ,Peer ,osPriorityNormal ,0,128);
	osThreadDef (duoji ,duoji ,osPriorityNormal ,0,128);
	osThreadDef (Love ,Love ,osPriorityNormal ,0,128);
	osThreadCreate (osThread (LedD3 ),NULL );
	osThreadCreate (osThread (LedD2 ),NULL );
	osThreadCreate (osThread (stopAll),NULL );
	osThreadCreate (osThread (SensorTask),NULL );
	osThreadCreate (osThread (Peer),NULL );
	osThreadCreate (osThread (duoji),NULL );
	osThreadCreate (osThread (Love),NULL );
	
  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init(); 
  /* Start scheduler */
  osKernelStart();
 
  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void LedD3(void const *argument){
	for(;;)
	{	
			if(flag == 2)
			{
				HAL_GPIO_WritePin (GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
				osDelay(250);
				HAL_GPIO_WritePin (GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
				osDelay(250);
			}else HAL_GPIO_WritePin (GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
	}	
}
void LedD2(void const *argument){
	for(;;)
	{	
			if(flag == 1)
			{
				HAL_GPIO_WritePin (GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
				osDelay(500);
				HAL_GPIO_WritePin (GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
				osDelay(500);
			}else HAL_GPIO_WritePin (GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
	}	
}

void SensorTask(void const *argument)
{
    for(;;)
    {
			if(flag ==4)
			{
				temp0 = (float)adcx[0]*(3.3/4096); 
        temp1 = (float)adcx[1]*(3.3/4096);
        temp2 = (float)adcx[2]*(3.3/4096);
			  temp3 = (float)adcx[3]*(3.3/4096);

        
        if(temp0 > 0.8 |temp1 > 2.5 ) alarm = 1;
        else alarm = 0;
        printf("Gas:%f light:%f Alarm: %d \n", temp0,temp1,alarm);

        osDelay(200); // 50~100ms ????		
			}
        
    }
}

void Peer(void const *argument)
{
    int melody[] = {
        262, 294, 330, 349, 392, 392, 392,
        294, 330, 349, 392, 440, 440, 440,
        392, 330, 294, 262, 330, 294, 262
    };
    int duration[] = {
        150,150,150,150,300,100,100,
        150,150,150,150,300,100,100,
        200,150,150,150,200,200,400
    };
    int length = sizeof(melody)/sizeof(melody[0]);

    for(;;)
    {
        if(flag == 1 || flag == 3 || alarm==1 )
        {
            for(int i = 0; i < length; i++)
            {
                int freq = melody[i];
                int time_ms = duration[i];
                // ???????????
                int half_period_us = 500000 / freq;

                uint32_t start = HAL_GetTick();
                while((HAL_GetTick() - start) < time_ms)
                {
                    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_SET);
                    for(volatile int j=0; j<half_period_us*10; j++); // ?????
                    HAL_GPIO_WritePin(GPIOG, GPIO_PIN_6, GPIO_PIN_RESET);
                    for(volatile int j=0; j<half_period_us*10; j++);
                }

                osDelay(20); // ???????
            }
        }
        else
        {
            osDelay(50);
        }
    }
}








void stopAll(void const *argument){
	for(;;)
	{	
			if(flag == 0)
			{
				HAL_GPIO_WritePin (GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
				osDelay(500);
				HAL_GPIO_WritePin (GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
			}
	}	
}

void duoji(void const *argument){
	for(;;)
	{	
			if(flag == 2)
			{
				for(int i = 600;i < 2400;i++){
					__HAL_TIM_SetCompare(&htim12, TIM_CHANNEL_1,i);
					osDelay(1);
				}
			for(int i = 2400;i > 600;i--){
					__HAL_TIM_SetCompare(&htim12,TIM_CHANNEL_1,i);
					osDelay(1);
				}		
			}
	}	
}

void Love(void const *argument){

	for(;;)
	{	
		if(flag==3){
			printf("liuaji");
		  HAL_Delay(1000);
		}
	}	
}

int fputc(int ch, FILE *f)
{ 
  uint8_t tmp[1]={0};
	tmp[0] = (uint8_t)ch;
	HAL_UART_Transmit(&huart1,tmp,1,10);	
	return ch;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	
    if (GPIO_Pin == GPIO_PIN_9)
    {
        flag ++ ;
    }
		else if(GPIO_Pin == GPIO_PIN_11)
		{
				flag = 0 ;
		}
		printf("PIN:%d,flag = %d",GPIO_Pin,flag);
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
