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

#define YELLOW_PIN                              GPIO_PIN_5
#define YELLOW_GPIO_PORT                        GPIOB

#define SOFTWARE_DEBUG 1

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
extern uint32_t Task_LD2_Profiler, YELLOW_Profiler, Default_Profiler;
extern volatile int freemem;
extern char pcWriteBuffer[512];
double result;

/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId LedTask1Handle;
osThreadId myTask03Handle;
osThreadId performTrigonomHandle;
osThreadId generatePrimeNuHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(const void *argument);
void LedTaskFunc(const void *argument);
void YellowLED(const void *argument);
void performTrigonometricCalculations_Func(const void *argument);
void generatePrimeNumbers_Func(const void *argument);

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
    result = sin(i) + cos(i);
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
void generatePrimeNumbers() {
  const int maxNumber = 100000;
  int count = 0;

  for (int number = 2; number <= maxNumber; number++) {
    if (isPrime(number)) {
      count++;
    }
  }
}

/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void) {

}

__weak unsigned long getRunTimeCounterValue(void) {
  return 0;
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
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of LedTask1 */
  osThreadDef(LedTask1, LedTaskFunc, osPriorityNormal, 0, 128);
  LedTask1Handle = osThreadCreate(osThread(LedTask1), NULL);

  /* definition and creation of myTask03 */
  osThreadDef(myTask03, YellowLED, osPriorityNormal, 0, 128);
  myTask03Handle = osThreadCreate(osThread(myTask03), NULL);

  /* definition and creation of performTrigonom */
  osThreadDef(performTrigonom, performTrigonometricCalculations_Func,
      osPriorityIdle, 0, 128);
  performTrigonomHandle = osThreadCreate(osThread(performTrigonom), NULL);

  /* definition and creation of generatePrimeNu */
  osThreadDef(generatePrimeNu, generatePrimeNumbers_Func, osPriorityIdle, 0,
      128);
  generatePrimeNuHandle = osThreadCreate(osThread(generatePrimeNu), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(const void *argument) {
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for (;;) {
    Default_Profiler++;
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

/* USER CODE BEGIN Header_LedTaskFunc */
/**
 * @brief Function implementing the LedTask1 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_LedTaskFunc */
void LedTaskFunc(const void *argument) {
  /* USER CODE BEGIN LedTaskFunc */
  /* Infinite loop */
  for (;;) {
    Task_LD2_Profiler++;
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
    vTaskDelay(LD2_Period);
    freemem = xPortGetFreeHeapSize();
    vTaskList(pcWriteBuffer);

  }
  /* USER CODE END LedTaskFunc */
}

/* USER CODE BEGIN Header_YellowLED */
/**
 * @brief Function implementing the myTask03 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_YellowLED */
void YellowLED(const void *argument) {
  /* USER CODE BEGIN YellowLED */
  /* Infinite loop */
  for (;;) {
    YELLOW_Profiler++;
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_8);
    HAL_GPIO_TogglePin(YELLOW_GPIO_PORT, YELLOW_PIN);
    vTaskDelay(YELLOW_Period);

  }
  /* USER CODE END YellowLED */
}

/* USER CODE BEGIN Header_performTrigonometricCalculations_Func */
/**
 * @brief Function implementing the performTrigonom thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_performTrigonometricCalculations_Func */
void performTrigonometricCalculations_Func(const void *argument) {
  /* USER CODE BEGIN performTrigonometricCalculations_Func */
  /* Infinite loop */
  for (;;) {

    performTrigonometricCalculations();
    vTaskDelay(1);
  }
  /* USER CODE END performTrigonometricCalculations_Func */
}

/* USER CODE BEGIN Header_generatePrimeNumbers_Func */
/**
 * @brief Function implementing the generatePrimeNu thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_generatePrimeNumbers_Func */
void generatePrimeNumbers_Func(const void *argument) {
  /* USER CODE BEGIN generatePrimeNumbers_Func */
  /* Infinite loop */
  for (;;) {
    generatePrimeNumbers();
    vTaskDelay(1);
  }
  /* USER CODE END generatePrimeNumbers_Func */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

