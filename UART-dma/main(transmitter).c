/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : UART Transmitter (Polling / Interrupt / DMA)
  ******************************************************************************
  */
/* USER CODE END Header */
#include "main.h"

/* Private define ------------------------------------------------------------*/
#define UART_POLLING   1
#define UART_INTERRUPT 2
#define UART_DMA       3
#define UART_MODE      UART_DMA   

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;
uint8_t tx_data[20]; // buffer untuk mengirim data

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN 0 */
// Callback saat transmit selesai untuk interrupt/DMA
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); // LED toggle saat TX selesai
    }
}

// isi buffer sesuai mode
void fill_tx_buffer(void)
{
    if(UART_MODE == UART_POLLING)
        memset(tx_data, 'P', sizeof(tx_data));
    else if(UART_MODE == UART_INTERRUPT)
        memset(tx_data, 'I', sizeof(tx_data));
    else if(UART_MODE == UART_DMA)
        memset(tx_data, 'D', sizeof(tx_data));
}
/* USER CODE END 0 */

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_USART1_UART_Init();

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // Inisialisasi LED

    // Hentikan DMA / interrupt agar tidak bentrok saat polling
    HAL_UART_Abort_IT(&huart1);
    HAL_UART_DMAStop(&huart1);

    /* Infinite loop */
    while(1)
    {
        fill_tx_buffer(); // update buffer sesuai mode

        if(UART_MODE == UART_POLLING)
        {
            HAL_UART_Transmit(&huart1, tx_data, sizeof(tx_data), HAL_MAX_DELAY);
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
        }
        else if(UART_MODE == UART_INTERRUPT)
        {
            HAL_UART_Transmit_IT(&huart1, tx_data, sizeof(tx_data));
            // tunggu hingga transmit selesai sebelum next loop
            while(huart1.gState != HAL_UART_STATE_READY) { }
        }
        else if(UART_MODE == UART_DMA)
        {
            HAL_UART_Transmit_DMA(&huart1, tx_data, sizeof(tx_data));
            // tunggu hingga transmit selesai sebelum next loop
            while(huart1.gState != HAL_UART_STATE_READY) { }
        }

        HAL_Delay(1000);
    }
}

/* System Clock Configuration */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|
                                  RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

/* USART1 Initialization */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if(HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

/* DMA Initialization */
static void MX_DMA_Init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    // TX DMA
    hdma_usart1_tx.Instance = DMA1_Channel4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_usart1_tx);
    __HAL_LINKDMA(&huart1, hdmatx, hdma_usart1_tx);

    // Enable DMA IRQ
    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
}

/* GPIO Initialization */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

/* Error Handler */
void Error_Handler(void)
{
    __disable_irq();
    while(1) { }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) { }
#endif
