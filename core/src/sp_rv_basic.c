/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
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
#include "rv_common.h"
#include "bearlyml23.h"
#include <inttypes.h>
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STR1(x) #x
#ifndef STR
#define STR(x) STR1(x)
#endif

#define CAT_(A, B) A##B
#define CAT(A, B) CAT_(A, B)

#define ROCC_INSTRUCTION_R_R_R(x, rd, rs1, rs2, func7)                                   \
    {                                                                                    \
        asm volatile(                                                                    \
            ".insn r " STR(CAT(CUSTOM_, x)) ", " STR(0x7) ", " STR(func7) ", %0, %1, %2" \
            : "=r"(rd)                                                                   \
            : "r"(rs1), "r"(rs2));                                                       \
    }

#define ROCC_INSTRUCTION_0_R_R(x, rs1, rs2, func7)                                       \
    {                                                                                    \
        asm volatile(                                                                    \
            ".insn r " STR(CAT(CUSTOM_, x)) ", " STR(0x3) ", " STR(func7) ", x0, %0, %1" \
            :                                                                            \
            : "r"(rs1), "r"(rs2));                                                       \
    }
#define CACHELINE 64

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static size_t n_cores = 4;
static void __attribute__((noinline)) barrier()
{
    static volatile int sense;
    static volatile int count;
    static __thread int threadsense;

    __sync_synchronize();

    threadsense = !threadsense;
    if (__sync_fetch_and_add(&count, 1) == n_cores - 1)
    {
        count = 0;
        sense = threadsense;
    }
    else
        while (sense != threadsense)
            ;

    __sync_synchronize();
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(int argc, char **argv)
{
    /* USER CODE BEGIN 1 */
    uint8_t counter = 0;

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    HAL_RCC_InitSystemClock();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    /* USER CODE BEGIN 2 */

    // // set up GPIO registers
    // GPIO_InitTypeDef GPIO_init_config;
    // GPIO_init_config.mode = GPIO_MODE_OUTPUT;
    // GPIO_init_config.pull = GPIO_PULL_NONE;
    // GPIO_init_config.drive_strength = GPIO_DS_STRONG;
    // HAL_GPIO_init(GPIOA, &GPIO_init_config, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    // // set up UART registers
    // UART_InitTypeDef UART_init_config;
    // UART_init_config.baudrate = 115200;
    // UART_init_config.mode = UART_MODE_TX_RX;
    // UART_init_config.stopbits = UART_STOPBITS_2;
    // HAL_UART_init(UART0, &UART_init_config);
    SET_BITS(UART0->TXCTRL, UART_TXCTRL_TXEN_MSK);

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    // while (1)
    // {
    //     uint64_t mhartid = READ_CSR("mhartid");
    //     printf("Hello world from hart %d: %d\n", mhartid, counter);
    //     counter += 1;
    //     /* USER CODE END WHILE */
    // }
    /* USER CODE BEGIN 3 */

    RCC->TILE0_RESET = 0;
    RCC->TILE1_RESET = 0;
    RCC->TILE2_RESET = 0;
    RCC->TILE3_RESET = 0;
    uint64_t mhartid = READ_CSR("mhartid");

    while (1) {
        HAL_delay(mhartid * 500);
        char buf[128];
        sprintf(buf, "Core %d is awake!\n", READ_CSR("mhartid"));
        HAL_UART_transmit(UART0, (uint8_t *)buf, strlen(buf), 100);

        for (size_t hart = 0; hart < n_cores; hart++)
        {
            if ((hart == mhartid) && (mhartid == 2 || mhartid == 3))
            {
                sprintf(buf, "\nTesting at hartid: %lu\n", mhartid);
                HAL_UART_transmit(UART0, (uint8_t *)buf, strlen(buf), 100);

                int8_t sparse_row[] = {-7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                int8_t sparse_row_size = 8;

                __attribute__((aligned(CACHELINE))) int8_t dense_matrix[] = {
                    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
                    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1
                };

                int8_t dense_matrix_size = 8;

                int16_t output_buf[100];

                // measure time taken for wordload
                uint64_t time1 = HAL_getTick();
                for (size_t i = 0; i < 100; i++) {
                    ROCC_INSTRUCTION_0_R_R(0, sparse_row, sparse_row_size, 0);
                    ROCC_INSTRUCTION_0_R_R(0, dense_matrix, dense_matrix_size, 1);
                    ROCC_INSTRUCTION_0_R_R(0, output_buf, output_buf, 2);
                    asm volatile("fence");
                };
                uint64_t time2 = HAL_getTick() - time1;
                sprintf(buf, "\nTime taken: %lu\n", time2);
                HAL_UART_transmit(UART0, (uint8_t *)buf, strlen(buf), 100);

                // Since Rocc is issuing L2 requests now, it is actually unaware of Fence Instruction
                // So it's safe to to wait for many cycles before checking the result
                // But you still can issue the next acceleration instructions
                for (int i = 0; i < 100; i++)
                {
                    ;
                }


                // first output result should be -7
                sprintf(buf, "first value of output is %d, should be -7\n", output_buf[0]);
                HAL_UART_transmit(UART0, (uint8_t *)buf, strlen(buf), 100);

                if (output_buf[0] != -7)
                {
                    sprintf(buf, "\nTest Failed at core %d\n", mhartid);
                    HAL_UART_transmit(UART0, (uint8_t *)buf, strlen(buf), 100);

                    sprintf(buf, "got %d", output_buf[0]);
                    HAL_UART_transmit(UART0, (uint8_t *)buf, strlen(buf), 100);
                }
                else
                {
                    sprintf(buf, "\nPASSED! at core %lu\n", mhartid);
                    HAL_UART_transmit(UART0, (uint8_t *)buf, strlen(buf), 100);
                }
            }
            barrier();
        }
    }

    /* USER CODE END 3 */
}

/*
 * Main function for secondary harts
 *
 * Multi-threaded programs should provide their own implementation.
 */
void __attribute__((weak, noreturn)) __main(void)
{
    while (1)
    {
        asm volatile("wfi");
    }
}




