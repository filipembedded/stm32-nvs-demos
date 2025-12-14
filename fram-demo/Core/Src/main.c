/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "port_stm32.h"
#include "fram.h"
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
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
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
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  
  // Initialize FRAM instance
  PortContext_TypeDef port_context;
  port_context.cs_pin = GPIO_PIN_4;
  port_context.cs_port = GPIOA;
  port_context.hspi = &hspi1;

  FRAM_Instance_TypeDef fram;
  fram.context = &port_context;
  fram.spi_chip_select = spi_chip_select;
  fram.spi_chip_deselect = spi_chip_deselect;
  fram.spi_write = spi_write;
  fram.spi_read = spi_read;

  // Test variables
  FRAM_Status_TypeDef status;
  uint8_t test_passed = 0;
  uint8_t test_failed = 0;
  
  // ===== TEST 1: Read Status Register =====
  uint8_t status_reg = 0;
  status = FRAM_ReadStatusReg(&fram, status_reg);
  if (status == FRAM_STATUS_SUCCESS) {
    test_passed++;
  } else {
    test_failed++;
  }

  // ===== TEST 2: Single Byte Write/Read at Address 0x0000 =====
  uint8_t write_byte = 0xAA;
  uint8_t read_byte = 0x00;
  
  status = FRAM_Write(&fram, 0x0000, &write_byte, 1);
  if (status != FRAM_STATUS_SUCCESS) test_failed++;
  
  status = FRAM_Read(&fram, 0x0000, &read_byte, 1);
  if (status == FRAM_STATUS_SUCCESS && read_byte == write_byte) {
    test_passed++;
  } else {
    test_failed++;
  }

  // ===== TEST 3: Multi-byte Write/Read at Address 0x0000 =====
  uint8_t write_data[] = "Hello FRAM!";
  uint8_t read_data[32] = {0};
  
  status = FRAM_Write(&fram, 0x0000, write_data, sizeof(write_data));
  if (status != FRAM_STATUS_SUCCESS) test_failed++;
  
  status = FRAM_Read(&fram, 0x0000, read_data, sizeof(write_data));
  if (status == FRAM_STATUS_SUCCESS) {
    uint8_t match = 1;
    for (uint8_t i = 0; i < sizeof(write_data); i++) {
      if (read_data[i] != write_data[i]) {
        match = 0;
        break;
      }
    }
    if (match) {
      test_passed++;
    } else {
      test_failed++;
    }
  } else {
    test_failed++;
  }

  // ===== TEST 4: Write/Read at Different Address (0x0100) =====
  uint8_t test4_write[] = "Address 0x0100";
  uint8_t test4_read[32] = {0};
  
  status = FRAM_Write(&fram, 0x0100, test4_write, sizeof(test4_write));
  if (status != FRAM_STATUS_SUCCESS) test_failed++;
  
  status = FRAM_Read(&fram, 0x0100, test4_read, sizeof(test4_write));
  if (status == FRAM_STATUS_SUCCESS) {
    uint8_t match = 1;
    for (uint8_t i = 0; i < sizeof(test4_write); i++) {
      if (test4_read[i] != test4_write[i]) {
        match = 0;
        break;
      }
    }
    if (match) {
      test_passed++;
    } else {
      test_failed++;
    }
  } else {
    test_failed++;
  }

  // ===== TEST 5: Write/Read at Higher Address (0x1234) =====
  uint8_t test5_write[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  uint8_t test5_read[8] = {0};
  
  status = FRAM_Write(&fram, 0x1234, test5_write, sizeof(test5_write));
  if (status != FRAM_STATUS_SUCCESS) test_failed++;
  
  status = FRAM_Read(&fram, 0x1234, test5_read, sizeof(test5_write));
  if (status == FRAM_STATUS_SUCCESS) {
    uint8_t match = 1;
    for (uint8_t i = 0; i < sizeof(test5_write); i++) {
      if (test5_read[i] != test5_write[i]) {
        match = 0;
        break;
      }
    }
    if (match) {
      test_passed++;
    } else {
      test_failed++;
    }
  } else {
    test_failed++;
  }

  // ===== TEST 6: Verify Previous Data at 0x0000 Still Intact =====
  uint8_t verify_data[32] = {0};
  status = FRAM_Read(&fram, 0x0000, verify_data, sizeof(write_data));
  if (status == FRAM_STATUS_SUCCESS) {
    uint8_t match = 1;
    for (uint8_t i = 0; i < sizeof(write_data); i++) {
      if (verify_data[i] != write_data[i]) {
        match = 0;
        break;
      }
    }
    if (match) {
      test_passed++;
    } else {
      test_failed++;
    }
  } else {
    test_failed++;
  }

  // ===== TEST 7: Write/Read at End of 64KB Range (0xFFF0) =====
  uint8_t test7_write[] = "END_TEST";
  uint8_t test7_read[16] = {0};
  
  status = FRAM_Write(&fram, 0xFFF0, test7_write, sizeof(test7_write));
  if (status != FRAM_STATUS_SUCCESS) test_failed++;
  
  status = FRAM_Read(&fram, 0xFFF0, test7_read, sizeof(test7_write));
  if (status == FRAM_STATUS_SUCCESS) {
    uint8_t match = 1;
    for (uint8_t i = 0; i < sizeof(test7_write); i++) {
      if (test7_read[i] != test7_write[i]) {
        match = 0;
        break;
      }
    }
    if (match) {
      test_passed++;
    } else {
      test_failed++;
    }
  } else {
    test_failed++;
  }

  // ===== TEST 8: Pattern Test - Alternating 0x55/0xAA =====
  uint8_t pattern_write[16];
  uint8_t pattern_read[16] = {0};
  for (uint8_t i = 0; i < 16; i++) {
    pattern_write[i] = (i % 2) ? 0xAA : 0x55;
  }
  
  status = FRAM_Write(&fram, 0x0500, pattern_write, sizeof(pattern_write));
  if (status != FRAM_STATUS_SUCCESS) test_failed++;
  
  status = FRAM_Read(&fram, 0x0500, pattern_read, sizeof(pattern_write));
  if (status == FRAM_STATUS_SUCCESS) {
    uint8_t match = 1;
    for (uint8_t i = 0; i < sizeof(pattern_write); i++) {
      if (pattern_read[i] != pattern_write[i]) {
        match = 0;
        break;
      }
    }
    if (match) {
      test_passed++;
    } else {
      test_failed++;
    }
  } else {
    test_failed++;
  }

  // ===== ALL TESTS COMPLETE =====
  // Set breakpoint here to inspect test_passed and test_failed
  // Expected: test_passed = 8, test_failed = 0
  
  if (test_failed > 0) {
    Error_Handler();
  }
  
  // Blink LED to indicate success (toggle PA5 or your LED pin)
  while(1) {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(test_passed * 100); // Blink rate indicates number of passed tests
  }

  /* USER CODE END 2 */

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SPI1_CS_Pin */
  GPIO_InitStruct.Pin = SPI1_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
