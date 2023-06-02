/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : app_freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stm32g4xx_hal_uart.h>
#include <usart.h>
#include "SEGGER_RTT.h"
#include "SEGGER_SYSVIEW.h"
#include <string.h>

#define YELLOW_PIN                              GPIO_PIN_5
#define YELLOW_GPIO_PORT                        GPIOB
//#define SOFTWARE_DEBUG 1

#if SOFTWARE_DEBUG == 1

extern volatile int freemem;
extern volatile int minfreemem;
extern char pcWriteBuffer[512];

#endif

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
/* USER CODE BEGIN Variables */

extern uint32_t LD2_Period, YELLOW_Period;
extern uint32_t Task_LD2_Profiler, YELLOW_Profiler, Default_Profiler,
    TriProfiler, FibProfiler, MemProfiler;

extern HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *huart,
    const uint8_t *pData, uint16_t Size, uint32_t Timeout);
extern UART_HandleTypeDef huart3;

double resulttrigon;
uint32_t resultFibon;
uint32_t resultPrimeNumbers;

uint32_t count = 0;
uint32_t number;
char buffer[512];
uint16_t NumberOfTasks;

struct name_t {
  const char *profiler_LD2;
  const char *profiler_Yellow;
  const char *profiler_Default;
};

struct name_t profiler_name =
    { .profiler_LD2 = "Profiler_LD2", .profiler_Yellow = "Profiler_Yellow",
        .profiler_Default = "Profiler_Default" };

/******SEGGER_RTT add channel 1: log ************/
static uint8_t logBuffer[32];

// Change this to adjust the number of Fibonacci numbers to calculate
uint32_t fibonacciNumber = 3000;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask",
    .priority = (osPriority_t) osPriorityNormal, .stack_size = 128 * 4 };
/* Definitions for LD2_Task */
osThreadId_t LD2_TaskHandle;
const osThreadAttr_t LD2_Task_attributes = { .name = "LD2_Task", .priority =
    (osPriority_t) osPriorityNormal, .stack_size = 128 * 4 };
/* Definitions for LEDYellow_Task */
osThreadId_t LEDYellow_TaskHandle;
const osThreadAttr_t LEDYellow_Task_attributes = { .name = "LEDYellow_Task",
    .priority = (osPriority_t) osPriorityNormal, .stack_size = 128 * 4 };
/* Definitions for Trigonometry */
osThreadId_t TrigonometryHandle;
const osThreadAttr_t Trigonometry_attributes = { .name = "Trigonometry",
    .priority = (osPriority_t) osPriorityAboveNormal1, .stack_size = 128 * 4 };
/* Definitions for TaskMemory */
osThreadId_t TaskMemoryHandle;
const osThreadAttr_t TaskMemory_attributes = { .name = "TaskMemory", .priority =
    (osPriority_t) osPriorityNormal, .stack_size = 128 * 4 };
/* Definitions for Fibonachi */
osThreadId_t FibonachiHandle;
const osThreadAttr_t Fibonachi_attributes = { .name = "Fibonachi", .priority =
    (osPriority_t) osPriorityNormal, .stack_size = 128 * 4 };

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void LD2TaskFunc(void *argument);
void YellowLED_Func(void *argument);
void performTrigonometricCalculations_Func(void *argument);
void TaskMemory_Func(void *argument);
void Fibonachi_Func(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);

/* USER CODE BEGIN 1 */

// Function to perform trigonometric calculations
void performTrigonometricCalculations() {
  const int numIterations = 10000;

  for (int i = 0; i < numIterations; i++) {
    // Perform trigonometric calculation
    resulttrigon = sin(i) + cos(i) / tan(i);
  }
}

// Function to check if a number is prime
bool isPrime(int number) {
  if (number <= 1) {
    return false;
  }

  for (int i = 2; i * i <= number; i++) {
    if (number % i == 0) {
      return false;
    }
  }

  return true;
}

// Function to generate prime numbers
uint32_t generatePrimeNumbers() {
  const uint64_t maxNumber = 1000000000;
  size_t count = 0;

  for (number = 2; number <= maxNumber; number++) {
    if (isPrime(number)) {
      //   printf("%ld\n", number);
      count++;
    }
  }
  return isPrime(number);
}

// Function to calculate the Fibonacci sequence
uint32_t fibonacci(uint32_t n) {
  if (n == 0) {
    return 0;
  } else if (n == 1) {
    return 1;
  } else {
    uint32_t a = 0;
    uint32_t b = 1;
    uint32_t c;

    for (uint32_t i = 2; i <= n; ++i) {
      c = a + b;
      a = b;
      b = c;
    }

    return b;
  }
}

//extern int _write(int file, char *ptr, int len);
//int _write(int file, char *ptr, int len) {
//  // Ваш код для передачи данных через UART
//  // Например, используйте HAL_UART_Transmit() для отправки данных
//  // Что-то вроде:
//  HAL_UART_Transmit(&huart1, (uint8_t*) ptr, len, HAL_MAX_DELAY);
//
//  // Верните количество отправленных байтов
//  return len;
//}

/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void) {

}
__weak unsigned long getRunTimeCounterValue(void) {
  return 0;
}

void vPrintStringAndNumber(uint32_t value, const char *profilerName) {
  snprintf(buffer, sizeof(buffer), "Profiler: %s, Value: %ld\n", profilerName,
      value);
  HAL_UART_Transmit(&huart3, (uint8_t*) buffer, strlen(buffer), HAL_MAX_DELAY);
}

/* USER CODE END 1 */

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
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
      &defaultTask_attributes);

  /* creation of LD2_Task */
  LD2_TaskHandle = osThreadNew(LD2TaskFunc, NULL, &LD2_Task_attributes);

  /* creation of LEDYellow_Task */
  LEDYellow_TaskHandle = osThreadNew(YellowLED_Func, NULL,
      &LEDYellow_Task_attributes);

  /* creation of Trigonometry */
  TrigonometryHandle = osThreadNew(performTrigonometricCalculations_Func, NULL,
      &Trigonometry_attributes);

  /* creation of TaskMemory */
  TaskMemoryHandle = osThreadNew(TaskMemory_Func, NULL, &TaskMemory_attributes);

  /* creation of Fibonachi */
  FibonachiHandle = osThreadNew(Fibonachi_Func, NULL, &Fibonachi_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  vTaskStartScheduler();
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument) {
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  SEGGER_RTT_Init();
#if SOFTWARE_DEBUG == 1

  SEGGER_RTT_ConfigUpBuffer(1, "Log", logBuffer, sizeof(logBuffer),
  SEGGER_RTT_MODE_NO_BLOCK_TRIM);
//  SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0,
//  SEGGER_RTT_MODE_NO_BLOCK_TRIM);
  SEGGER_RTT_SetTerminal(0);
  SEGGER_RTT_printf(0, "DefaultTask\r\n");
#endif
  for (;;) {
    Default_Profiler++;
    vPrintStringAndNumber(Default_Profiler, profiler_name.profiler_Default);
#if SOFTWARE_DEBUG == 1
//    SEGGER_RTT_SetTerminal(0);
//    SEGGER_RTT_printf(0, "Default_Profiler = %d\n", Default_Profiler);
#endif
    if (LD2_Period == 100) {
      LD2_Period = 1000;
      YELLOW_Period = LD2_Period / 3;
    } else
      LD2_Period = 100;
    YELLOW_Period = LD2_Period * 3;
    vTaskDelay(5000);
  }

  /* USER CODE END StartDefaultTask */
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* USER CODE BEGIN Header_LD2TaskFunc */
/**
 * @brief Function implementing the LD2_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_LD2TaskFunc */
void LD2TaskFunc(void *argument) {
  /* USER CODE BEGIN LD2TaskFunc */
  /* Infinite loop */
  for (;;) {
    Task_LD2_Profiler++;
    vPrintStringAndNumber(Task_LD2_Profiler, profiler_name.profiler_LD2);
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    vTaskDelay(LD2_Period);
  }
  /* USER CODE END LD2TaskFunc */
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* USER CODE BEGIN Header_YellowLED_Func */
/**
 * @brief Function implementing the LEDYellow_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_YellowLED_Func */
void YellowLED_Func(void *argument) {
  /* USER CODE BEGIN YellowLED_Func */
  /* Infinite loop */
  for (;;) {
    YELLOW_Profiler++;
    vPrintStringAndNumber(YELLOW_Profiler, profiler_name.profiler_Yellow);
    HAL_GPIO_TogglePin(YELLOW_GPIO_PORT, YELLOW_PIN);
    vTaskDelay(YELLOW_Period);
  }
  /* USER CODE END YellowLED_Func */
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* USER CODE BEGIN Header_performTrigonometricCalculations_Func */
/**
 * @brief Function implementing the performTrigonom thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_performTrigonometricCalculations_Func */
void performTrigonometricCalculations_Func(void *argument) {
  /* USER CODE BEGIN performTrigonometricCalculations_Func */
  /* Infinite loop */
  for (;;) {
    TriProfiler++;
    performTrigonometricCalculations();
    vTaskDelay(1);
  }
  /* USER CODE END performTrigonometricCalculations_Func */
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* USER CODE BEGIN Header_TaskMemory_Func */
/**
 * @brief Function implementing the TaskMemory thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_TaskMemory_Func */
void TaskMemory_Func(void *argument) {
  /* USER CODE BEGIN TaskMemory_Func */
  /* Infinite loop */
  SEGGER_SYSVIEW_Conf(); /* Configure and initialize SystemView */
  for (;;) {
#if SOFTWARE_DEBUG == 1
//    /* add channel 1: log */
//    SEGGER_RTT_ConfigUpBuffer(1, "Log", logBuffer, sizeof(logBuffer),
//    SEGGER_RTT_MODE_NO_BLOCK_TRIM);
    MemProfiler++;
    freemem = xPortGetFreeHeapSize();
    minfreemem = xPortGetMinimumEverFreeHeapSize();
    NumberOfTasks = uxTaskGetNumberOfTasks();
    //vTaskGetRunTimeStats(pcWriteBuffer);
    vTaskList(pcWriteBuffer);
    SEGGER_RTT_SetTerminal(0);
    SEGGER_RTT_WriteString(0, pcWriteBuffer);
    SEGGER_RTT_printf(0, "___________________________\r\n");

#endif
    vTaskDelay(300);
  }
  /* USER CODE END TaskMemory_Func */
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/* USER CODE BEGIN Header_Fibonachi_Func */
/**
 * @brief Function implementing the Fibonachi thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Fibonachi_Func */
void Fibonachi_Func(void *argument) {
  /* USER CODE BEGIN Fibonachi_Func */
  /* Infinite loop */
  for (;;) {
    FibProfiler++;
    resultFibon = fibonacci(fibonacciNumber);
    //    printf("Fibonacci number at position %ld: %ld\n", fibonacciNumber,
    //        resultFibon);
    vTaskDelay(1);
  }
  /* USER CODE END Fibonachi_Func */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

