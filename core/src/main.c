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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#define STR1(x) #x
#ifndef STR
#define STR(x) STR1(x)
#endif

#define CAT_(A, B) A##B
#define CAT(A, B)  CAT_(A, B)

#define ROCC_INSTRUCTION_R_R_R(x, rd, rs1, rs2, func7)                         \
  {                                                                            \
    asm volatile(".insn r " STR(CAT(CUSTOM_, x)) ", " STR(0x7) ", " STR(       \
                     func7) ", %0, %1, %2"                                     \
                 : "=r"(rd)                                                    \
                 : "r"(rs1), "r"(rs2));                                        \
  }

#define ROCC_INSTRUCTION_0_R_R(x, rs1, rs2, func7)                             \
  {                                                                            \
    asm volatile(".insn r " STR(CAT(CUSTOM_, x)) ", " STR(0x3) ", " STR(       \
                     func7) ", x0, %0, %1"                                     \
                 :                                                             \
                 : "r"(rs1), "r"(rs2));                                        \
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

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void LL_UART_transmit(UART_TypeDef *UARTx, const uint8_t *data, uint16_t size, uint32_t timeout) {
  while (size > 0) {
    while (READ_BITS(UARTx->TXDATA, UART_TXDATA_FULL_MSK)) {
      // return TIMEOUT;
    }
    UARTx->TXDATA = *data;
    data += sizeof(uint8_t);
    size -= 1;
  }
  return OK;
}

void *memcpy(void *dest, const void *src, size_t n) {
  for (size_t i = 0; i < n; i++) {
    ((char *)dest)[i] = ((char *)src)[i];
  }
}

void intToAscii(uint64_t num, char str[]) {
  int i = 0;

  // Handle the case of 0 separately
  if (num == 0) {
    str[i++] = '0';
  } else {
    // Handle negative numbers
    if (num < 0) {
      str[i++] = '-';
      num = -num;
    }

    // Extract digits one by one
    int temp = num;
    while (temp > 0) {
      uint64_t digit = temp % 10;
      str[i++] = '0' + digit;
      temp /= 10;
    }

    // Reverse the string to get the correct order
    int start = (str[0] == '-') ? 1 : 0;
    int end = i - 1;
    while (start < end) {
      char tempChar = str[start];
      str[start++] = str[end];
      str[end--] = tempChar;
    }
  }

  // Null-terminate the string
  str[i] = '\0';
}

void accel_model(int8_t *sparse_row, int8_t sparse_row_size,
                 int8_t *dense_matrix, int8_t dense_matrix_stride,
                 int8_t dense_matrix_size, int16_t *output_buf) {

  int32_t overflow_buf[64];

  for (int8_t i = 0; i < 64; i++) {
    overflow_buf[i] =
        sparse_row[0] *
        dense_matrix[(sparse_row[1] * dense_matrix_stride * dense_matrix_size) +
                     i];
  }
  for (int8_t i = 2; i < 2 * sparse_row_size; i += 2) {
    for (int8_t j = 0; j < 64; j++) {
      overflow_buf[j] =
          overflow_buf[j] +
          sparse_row[i] *
              dense_matrix[(sparse_row[i + 1] * dense_matrix_stride *
                            dense_matrix_size) +
                           j];
    }
  }
  for (int i = 0; i < 64; i++) {
    output_buf[i] = MAX(-32768, MIN(32767, overflow_buf[i]));
  }
}

char str[64] = "Start test.\n";
char str2[64] = "Starting accelerator.\n";
char fail[64] = "\nTest Failed\n";
char success[64] = "\nTest Success\n";
char debug1[64] = "\nSent Sparse rows\n";
char debug2[64] = "\nSent Dense rows\n";
char debug3[64] = "\nSent all accelerator intr.";
char debug4[64] = "\nCheck calculation time.\n";
char wait[64] = "\n\n";

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(int argc, char **argv) {
  /* USER CODE BEGIN 1 */
  uint8_t counter = 0;
  
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  /* USER CODE BEGIN 2 */

  // set up GPIO registers
  // GPIO_InitTypeDef GPIO_init_config;
  // GPIO_init_config.mode = GPIO_MODE_OUTPUT;
  // GPIO_init_config.pull = GPIO_PULL_NONE;
  // GPIO_init_config.drive_strength = GPIO_DS_STRONG;
  // HAL_GPIO_init(GPIOA, &GPIO_init_config, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

  // set up UART registers
  // UART_InitTypeDef UART_init_config;
  // UART_init_config.baudrate = 115200;
  // UART_init_config.mode = UART_MODE_TX_RX;
  // UART_init_config.stopbits = UART_STOPBITS_2;
  // HAL_UART_init(UART0, &UART_init_config);
  
  CLEAR_BITS(UART0->RXCTRL, UART_RXCTRL_RXEN_MSK);
  CLEAR_BITS(UART0->TXCTRL, UART_TXCTRL_TXEN_MSK);

  SET_BITS(UART0->RXCTRL, UART_RXCTRL_RXEN_MSK);
  SET_BITS(UART0->TXCTRL, UART_TXCTRL_TXEN_MSK);

  CLEAR_BITS(UART0->TXCTRL, UART_TXCTRL_NSTOP_MSK);
  CLEAR_BITS(UART0->TXCTRL, UART_STOPBITS_2);
  
  // baudrate setting
  // f_baud = f_sys / (div + 1)
  // UART0->DIV = (SYS_CLK_FREQ / 115200) - 1;
  

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  // RCC->TILE0_RESET = 0;
  // // RCC->TILE1_RESET = 0;
  RCC->TILE2_RESET = 0;
  RCC->TILE3_RESET = 0;
  while (1) {
    //uint64_t mhartid = READ_CSR("mhartid");
    // printf("Hello world from hart %d: %d\n", mhartid, counter);
    // LL_UART_transmit(UART0, (uint8_t *) mhartid, 64, 0);
    counter += 1;
    //LL_UART_transmit(UART0, str, 64, 0);

    // begin data section
    int8_t sparse_row[] = {7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int8_t sparse_row_size = 8;

    __attribute__((aligned(CACHELINE))) int8_t dense_matrix[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

    int8_t dense_matrix_size = 8;

    int16_t output_buf[100];
    // correct result: output_buf[0] == 7
    // end data section.

    uint64_t register1 = 0x0706050403020100;
    uint64_t register2 = 0x0f0e0d0c0b0a0908;

    LL_UART_transmit(UART0, str2, 64, 0);
    // measure time taken for wordload
    uint64_t time1 = CLINT->MTIME;
    for (size_t i = 0; i < 10; i++) {
      asm volatile("fence");
      ROCC_INSTRUCTION_0_R_R(0, dense_matrix, dense_matrix_size, 1);
      ROCC_INSTRUCTION_0_R_R(0, register1, register2, 4);
      ROCC_INSTRUCTION_0_R_R(0, sparse_row, sparse_row_size, 0);
      ROCC_INSTRUCTION_0_R_R(0, output_buf, output_buf, 2);
      asm volatile("fence");
      LL_UART_transmit(UART0, debug3, 64, 0);
    };
    uint64_t time2 = CLINT->MTIME - time1;
    //LL_UART_transmit(UART0, (uint8_t *)time2, 64, 100);

    int16_t result = output_buf[0];
    char result_char[64];
    intToAscii(result, result_char);
    LL_UART_transmit(UART0, result_char, 64, 0);

    // Since Rocc is issuing L2 requests now, it is actually unaware of
    // Fence Instruction So it's safe to to wait for many cycles before
    // checking the result But you still can issue the next acceleration
    // instructions
    for (int i = 0; i < 100; i++) {
      ;
    }

    // if (output_buf[0] != 176) {
    //   LL_UART_transmit(UART0, fail, 64, 100);

    //   // LL_UART_transmit(UART0, (uint8_t *)output_buf[0], 64, 100);
    // } else {
    //   LL_UART_transmit(UART0, success, 64, 100);
    // }
    /* USER CODE END WHILE */
  }
  /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
}

/*
 * Main function for secondary harts
 * 
 * Multi-threaded programs should provide their own implementation.
 */
void __attribute__((weak, noreturn)) __main(void) {
  while (1) {
    asm volatile("wfi");
    LL_UART_transmit(UART0, str2, 64, 0);
    //uint64_t mhartid = READ_CSR("mhartid");
    //LL_UART_transmit(UART0, (uint8_t *)mhartid, 64, 0);
    //LL_UART_transmit(UART0, success, 64, 0);
    //LL_UART_transmit(UART0, str, 64, 0);
    // data section
    int8_t sparse_row[] = {7, 0, 6, 1, 0, 2, 1, 0, 1, 2, 0, 0, 0, 0, 2, 0};
    int8_t sparse_row_size = 8;
    // 3*64
    __attribute__((aligned(CACHELINE))) int8_t dense_matrix[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    };

    int8_t dense_matrix_size = 8;

    int16_t output_buf[100];

    // data2
    // int8_t sparse_row[] = {127,  0, 127,  1, 127, 2, 127, 3, -127, 4, -127, 5,
    //                        -127, 6, -127, 7, 1,   0, 0,   0, 0,    0, 0,    0};

    // int8_t sparse_row_size = 12;

    // // // 10 x 64 dense_matrix
    // __attribute__((aligned(CACHELINE))) int8_t dense_matrix[] = {
    //     127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     127, 127, 127, 127, 127, 127, 127, 127, 0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0,
    //     0,   0,   0,   0,   0,   0,   0,   0,   0, 0, 0, 0, 0, 0, 0, 0

    // };

    // int8_t dense_matrix_size = 8;

    // int16_t output_buf[100];
    // output_buf[0] should be 127

    // end data section

    // test int to ascii translation is correct
    // uint64_t num = 280;
    // char translated_num[64];
    // intToAscii(num, translated_num);
    // LL_UART_transmit(UART0, translated_num, 64, 0);
    // LL_UART_transmit(UART0, wait, 64, 0);

    LL_UART_transmit(UART0, str2, 64, 0);
    // measure time taken for wordload
    uint64_t time1 = CLINT->MTIME;
    for (size_t i = 0; i < 50; i++) {
      asm volatile("fence");
      ROCC_INSTRUCTION_0_R_R(0, sparse_row, sparse_row_size, 0);
      // LL_UART_transmit(UART0, debug1, 64, 0);
      ROCC_INSTRUCTION_0_R_R(0, dense_matrix, dense_matrix_size, 1);
      // LL_UART_transmit(UART0, debug2, 64, 0);
      ROCC_INSTRUCTION_0_R_R(0, output_buf, output_buf, 2);
      asm volatile("fence");
    };

    uint64_t time2 = CLINT->MTIME - time1;
    //LL_UART_transmit(UART0, debug3, 64, 0);
    // LL_UART_transmit(UART0, debug4, 64, 0);

    char time_taken[64];
    intToAscii(time2, time_taken);
    LL_UART_transmit(UART0, time_taken, 64, 0);

    LL_UART_transmit(UART0, wait, 64, 0);

    // test CPU calculation time
    char output2[100];
    uint64_t time1_cpu = CLINT->MTIME;
    for (size_t i = 0; i < 50; i++) {
      asm volatile("fence");
      accel_model(sparse_row, sparse_row_size, dense_matrix, 8,
                  dense_matrix_size, output2);
      asm volatile("fence");
    }
    uint64_t time2_cpu = CLINT->MTIME - time1;
    char time_taken_cpu[64];
    intToAscii(time2_cpu, time_taken_cpu);
    LL_UART_transmit(UART0, time_taken_cpu, 64, 0);

    LL_UART_transmit(UART0, wait, 64, 0);
    // check relative time improvement
    char improvement_char[64];
    uint64_t improvement = time2_cpu / time2;
    intToAscii(improvement, improvement_char);
    LL_UART_transmit(UART0, improvement_char, 64, 0);

    //LL_UART_transmit(UART0, result, 64, 0);

    // Since Rocc is issuing L2 requests now, it is actually unaware of
    // Fence Instruction So it's safe to to wait for many cycles before
    // checking the result But you still can issue the next acceleration
    // instructions
    for (int i = 0; i < 100; i++) {
      ;
    }

    // char str[64] = "first value of output is %d, should be -7\n";
    // LL_UART_transmit(UART0, (uint8_t *)buf, strlen(buf), 100);

    if (output_buf[0] != 7) {
      LL_UART_transmit(UART0, fail, 64, 0);

      int16_t result = output_buf[0];
      char result_char[64];
      intToAscii(result, result_char);
      LL_UART_transmit(UART0, result_char, 64, 0);
    } else {
      LL_UART_transmit(UART0, success, 64, 0);
    }
  }
}
