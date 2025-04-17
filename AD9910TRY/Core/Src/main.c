/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define AD9910_DRC_Set HAL_GPIO_WritePin(GPIOG,GPIO_PIN_7,GPIO_PIN_SET)//PG7ΪDRC����  
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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
  /* USER CODE BEGIN 2 */

    //���ǲ�����
   AD9910_Init();																			// AD9910ʱ�ӵ����ú͸���DAC���� �Լ�AD9910��λ
    AD9910_Singal_Profile_Init();																	// ��ƵPROFILE��ʼ��
    AD9910_Singal_Profile_Set(0, 450000000, 0x3FFF, 0);									// �������Ҳ���Ϣ �� ͨ����0~7�� �� Ƶ�� ������ �� ��λ ��

    AD9910_RAM_Init();		// RAM���ܳ�ʼ��
    AD9910_RAM_ZB_Fre_Init();                                       // �ز�Ƶ�ʳ�ʼ��
    AD9910_RAM_ZB_Fre_Set(0);                                       // �����ز�Ƶ��
    AD9910_WAVE_RAM_AMP_W(2);                                       // д���ȵ�RAM							
    AD9910_RAM_CON_RECIR_AMP_R(10000);   //��л2021������

    //���Ҳ�����
//    AD9910_Init();                                                  // ��ʼ��AD9910
//    AD9910_Singal_Profile_Init();                                   // ��ƵPROFILE��ʼ��
//    AD9910_Singal_Profile_Set(0, 20000, 0x3FFF, 0);             // �������Ҳ���Ϣ �� ͨ����0~7�� �� Ƶ�� ������ �� ��λ �� 20KHZ

		//ɨƵģʽ
		//---------------------------------------------------------------------------------------------------------------------------------
//��������:void AD9910_DRG_Freq_set(u32 upper_limit , u32 lower_limit ,u32 dec_step , u32 inc_step , u16 neg_rate ,u16 pos_rate)
//��������:AD9910��DRG����------------------Ƶ��
//��ڲ���:		upper_limit										����Ƶ��							1Hz---450M
//						lower_limit										����Ƶ��							1Hz---450M       (����Ƶ��Ҫ��������Ƶ��)
//						dec_step											Ƶ�ʼӲ���						1Hz---450M					
//						inc_step											Ƶ�ʼ�����						1Hz---450M
//						neg_rate											Ƶ�ʼӵ�����(ʱ��)	0x0000----0xffff			  
//						pos_rate											Ƶ�ʼ�������(ʱ��)	0x0000----0xffff
//���ڲ���:��
//---------------------------------------------------------------------------------------------------------------------------------
//		AD9910_DRG_Fre_Init();																															// ����б�³�ʼ��------------Ƶ��
//									// ���ã�����Ƶ�ʣ�����Ƶ�ʣ�Ƶ�ʼӲ�����Ƶ�ʼ�������Ƶ�ʼӲ���ʱ�䣬Ƶ�ʼ�����ʱ�䣩
//		AD9910_DRG_Freq_set(10000000, 1, 1, 1, 60, 60);
//		
//		
//		AD9910_DRC_Set;
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
//    AD9910_WAVE_RAM_AMP_W(3);                                       // д���ȵ�RAM							
		/* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    		//���Ҳ�����
   // AD9910_Init();																									// ��ʼ��AD9910

    
    //
   
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
