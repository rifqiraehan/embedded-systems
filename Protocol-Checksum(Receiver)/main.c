/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : UART Receiver (Debug View via Live Expression)
  * @board          : STM32F103C8T6 (Blue Pill)
  ******************************************************************************
  */
#include "main.h"
#include <string.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
uint8_t rxFrame[7];     // Data frame yang diterima [SOF][D1..D5][CHK]
uint8_t data[5];        // Isi data utama
uint8_t SOF;            // Start of Frame
uint8_t rxChk;          // Checksum diterima
uint8_t calcChk;        // Checksum hasil hitung ulang
uint8_t validFrame;     // Status validasi frame (0 = salah, 1 = benar)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  while (1)
  {
    // Terima 7 byte frame
    HAL_UART_Receive(&huart1, rxFrame, 7, HAL_MAX_DELAY);

    // Pisahkan data
    SOF   = rxFrame[0];
    memcpy(data, &rxFrame[1], 5);
    rxChk = rxFrame[6];

    // Cek SOF
    if (SOF != 0xA5)
    {
      validFrame = 0;  // Frame tidak valid
      continue;        // lanjut tunggu frame berikut
    }

    // Hitung checksum
    calcChk = 0;
    for (int i = 0; i < 5; i++) calcChk += data[i];
    calcChk &= 0xFF;

    // Bandingkan checksum
    if (calcChk == rxChk)
      validFrame = 1;   // Frame valid
    else
      validFrame = 0;   // Frame rusak

    // --- Tempatkan breakpoint di sini ---
    // Saat breakpoint aktif, lihat nilai variabel di Live Expression:
    // rxFrame[], data[], SOF, rxChk, calcChk, validFrame
    __NOP(); // instruksi dummy supaya bisa diberi breakpoint

  } // end while
}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart1);
}

/* System Clock Configuration for STM32F103C8T6 @72MHz */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/* GPIO init */
static void MX_GPIO_Init(void)
{
  __HAL_RCC_GPIOA_CLK_ENABLE();
}
