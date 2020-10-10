/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "img.h"
#include "SSD1331.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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
CAN_RxHeaderTypeDef rxHeader;                //CAN Bus Transmit Header
CAN_TxHeaderTypeDef txHeader;                //CAN Bus Receive Header
uint8_t canRX[8] = {0, 0, 0, 0, 0, 0, 0, 0}; //CAN Bus Receive Buffer
CAN_FilterTypeDef canfil;                    //CAN Bus Filter
uint32_t canMailbox;                         //CAN Bus Mail box variable
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan1)
{
  HAL_CAN_GetRxMessage(hcan1, CAN_RX_FIFO0, &rxHeader, canRX); //Receive CAN bus message to canRX buffer
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);                      // toggle PA3 LED
}

void wait_and_clear() {
    HAL_Delay(1000);
    // clear display
    SSD1331_drawFrame(0,0,96,64, COLOR_BLACK, COLOR_BLACK);
    HAL_Delay(500);
}
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
  MX_CAN_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  canfil.FilterBank = 0;
  canfil.FilterMode = CAN_FILTERMODE_IDMASK;
  canfil.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfil.FilterIdHigh = 0;
  canfil.FilterIdLow = 0;
  canfil.FilterMaskIdHigh = 0;
  canfil.FilterMaskIdLow = 0;
  canfil.FilterScale = CAN_FILTERSCALE_32BIT;
  canfil.FilterActivation = ENABLE;
  canfil.SlaveStartFilterBank = 14;

  txHeader.DLC = 8; // Number of bites to be transmitted max- 8
  txHeader.IDE = CAN_ID_STD;
  txHeader.RTR = CAN_RTR_DATA;
  txHeader.StdId = 0x030;
  txHeader.ExtId = 0x02;
  txHeader.TransmitGlobalTime = DISABLE;

  HAL_CAN_ConfigFilter(&hcan, &canfil);                             //Initialize CAN Filter
  HAL_CAN_Start(&hcan);                                             //Initialize CAN Bus
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING); // Initi
  char buffer[40];
	char i;
	const char *str_inFlash ="avislab.com/blog";


  while (1)
  {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);

    // initialization
    SSD1331_init();

    wait_and_clear();

    // lines
    for (i = 0; i < 24; i++) {
    	SSD1331_drawLine(0, 0, i*4, 64, RGB(i*10,0,0));
    	SSD1331_drawLine(96, 0, i*4, 64, RGB(i*10,0,0));
    	HAL_Delay(20);
	}
    wait_and_clear();

    // circle
    SSD1331_drawCircle(48, 32, 5, COLOR_BLUE);
    HAL_Delay(50);
    SSD1331_drawCircle(48, 32, 10, COLOR_GOLDEN);
    HAL_Delay(50);
    SSD1331_drawCircle(48, 32, 15, COLOR_BLUE);
    HAL_Delay(50);
    SSD1331_drawCircle(48, 32, 20, COLOR_GOLDEN);
    HAL_Delay(50);
    SSD1331_drawCircle(48, 32, 25, COLOR_BLUE);
    HAL_Delay(50);
    SSD1331_drawCircle(48, 32, 30, COLOR_GOLDEN);
    wait_and_clear();

    // rect
    for (i = 0; i < 5; i++) {
    	SSD1331_drawFrame(i*5, i*5, 96-i*5, 64-i*5, COLOR_RED, COLOR_YELLOW);
    	HAL_Delay(20);
	}
    SSD1331_drawFrame(25, 25, 71, 39, COLOR_BLUE, COLOR_GREEN);
    wait_and_clear();

    // text FONT_1X
    SSD1331_SetXY(0,0);
    for (i = 33; i < 126; i++) {
    	SSD1331_Chr(FONT_1X, i, COLOR_BLUE, COLOR_BLACK);
    	SSD1331_XY_INK(FONT_1X);
	}
    wait_and_clear();

    // text FONT_2X
    SSD1331_SetXY(0,0);
    for (i = 33; i < 64; i++) {
    	SSD1331_Chr(FONT_2X, i, COLOR_BROWN, COLOR_BLACK);
    	SSD1331_XY_INK(FONT_2X);
	}
    wait_and_clear();

    // Numeric FONT_2X
    sprintf(buffer, "%s", "3758");
    SSD1331_SetXY(0, 0);
    SSD1331_Str(FONT_2X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLACK);
    wait_and_clear();

    // Numeric FONT_4X
    SSD1331_SetXY(0, 0);
    SSD1331_Str(FONT_4X,(unsigned char*)buffer, COLOR_WHITE, COLOR_BLACK);
    wait_and_clear();

    // Images
    SSD1331_IMG(IMG0, 0,0, 96,64);
    wait_and_clear();

    SSD1331_IMG(IMG1, 16,0, 64,64);
    wait_and_clear();

    // copy window
    SSD1331_IMG(IMG3, 32,16, 32,32);
    SSD1331_copyWindow(32,16,64,48, 0,0);
    wait_and_clear();

    // scrolling
    SSD1331_IMG(IMG2, 23,7, 50,50);
    SSD1331_setScrolling(Horizontal, 0, 64, 1);
    SSD1331_enableScrolling(TRUE);

    wait_and_clear();
    SSD1331_enableScrolling(FALSE);
    // text from Flash
    SSD1331_SetXY(0, 24);
    SSD1331_FStr(FONT_1X,(unsigned char*)str_inFlash, COLOR_BLUE, COLOR_BLACK);

    /*uint8_t csend[] = {0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01};
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
    if (HAL_CAN_AddTxMessage(&hcan, &txHeader, csend, &canMailbox) != HAL_OK)
    {
      Error_Handler();
    }*/
    /*HAL_Delay(300);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
    HAL_Delay(1000);*/
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
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
