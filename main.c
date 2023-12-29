#include "main.h"
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "canopennode.h"
#include "flash_data.h"



typedef StaticTask_t osStaticThreadDef_t;
typedef StaticEventGroup_t osStaticEventGroupDef_t;


/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
uint32_t defaultTaskBuffer[ 128 ];
osStaticThreadDef_t defaultTaskControlBlock;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .cb_mem = &defaultTaskControlBlock,
  .cb_size = sizeof(defaultTaskControlBlock),
  .stack_mem = &defaultTaskBuffer[0],
  .stack_size = sizeof(defaultTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for KeyboardTask */
osThreadId_t KeyboardTaskHandle;
uint32_t KeyboardTaskBuffer[ 128 ];
osStaticThreadDef_t KeyboardTaskControlBlock;
const osThreadAttr_t KeyboardTask_attributes = {
  .name = "KeyboardTask",
  .cb_mem = &KeyboardTaskControlBlock,
  .cb_size = sizeof(KeyboardTaskControlBlock),
  .stack_mem = &KeyboardTaskBuffer[0],
  .stack_size = sizeof(KeyboardTaskBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ProcessTask */
osThreadId_t ProcessTaskHandle;
uint32_t ProcessTaskBuffer[ 128 ];
osStaticThreadDef_t ProcessTaskControlBlock;
const osThreadAttr_t ProcessTask_attributes = {
  .name = "ProcessTask",
  .cb_mem = &ProcessTaskControlBlock,
  .cb_size = sizeof(ProcessTaskControlBlock),
  .stack_mem = &ProcessTaskBuffer[0],
  .stack_size = sizeof(ProcessTaskBuffer),
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for CanOpenPeriodic */
osThreadId_t CanOpenPeriodicHandle;
uint32_t CanOpenPeriodicBuffer[ 256 ];
osStaticThreadDef_t CanOpenPeriodicControlBlock;
const osThreadAttr_t CanOpenPeriodic_attributes = {
  .name = "CanOpenPeriodic",
  .cb_mem = &CanOpenPeriodicControlBlock,
  .cb_size = sizeof(CanOpenPeriodicControlBlock),
  .stack_mem = &CanOpenPeriodicBuffer[0],
  .stack_size = sizeof(CanOpenPeriodicBuffer),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for CanOpenProcess */
osThreadId_t CanOpenProcessHandle;
uint32_t CanOpenProcessBuffer[ 700 ];
osStaticThreadDef_t CanOpenProcessControlBlock;
const osThreadAttr_t CanOpenProcess_attributes = {
  .name = "CanOpenProcess",
  .cb_mem = &CanOpenProcessControlBlock,
  .cb_size = sizeof(CanOpenProcessControlBlock),
  .stack_mem = &CanOpenProcessBuffer[0],
  .stack_size = sizeof(CanOpenProcessBuffer),
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for xResetEvent */
osEventFlagsId_t xResetEventHandle;
osStaticEventGroupDef_t cResetEventControlBlock;
const osEventFlagsAttr_t xResetEvent_attributes = {
  .name = "xResetEvent",
  .cb_mem = &cResetEventControlBlock,
  .cb_size = sizeof(cResetEventControlBlock),
};

void StartDefaultTask(void *argument);
extern void vKeyboardTask(void *argument);
extern void vProcessTask(void *argument);
extern void vCanOpenPeriodicProcess(void *argument);
extern void vCanOpenProcess(void *argument);
CAN_HandleTypeDef  hcan;


int main( void )
{
	 SystemInit();
   vInit_DeviceConfig( );
	
	
   osKernelInitialize();
	
	 defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of KeyboardTask */
  KeyboardTaskHandle = osThreadNew(vKeyboardTask, NULL, &KeyboardTask_attributes);

  /* creation of ProcessTask */
  ProcessTaskHandle = osThreadNew(vProcessTask, NULL, &ProcessTask_attributes);

  /* creation of CanOpenPeriodic */
  CanOpenPeriodicHandle = osThreadNew(vCanOpenPeriodicProcess, NULL, &CanOpenPeriodic_attributes);

  /* creation of CanOpenProcess */
  CanOpenProcessHandle = osThreadNew(vCanOpenProcess, NULL, &CanOpenProcess_attributes);
	  vFDInit();
   vLedInit(TIMER1);
   vSetupKeyboard();
   vProceesInit();
	 hcan.can_periph = CAN0;
   vCanOpenInit(&hcan);
   vStartLed();

  /* USER CODE END RTOS_THREADS */

  /* Create the event(s) */
  /* creation of xResetEvent */
  xResetEventHandle = osEventFlagsNew(&xResetEvent_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

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
	
}


void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  vLedDriverStart();
  xEventGroupSetBits( xResetEventHandle, RESTART_DISABLE);
  for(;;)
  {
    xEventGroupWaitBits(xResetEventHandle,RESTART_DISABLE,pdFALSE,pdFALSE,portMAX_DELAY );
    osDelay(500);
	//	fwdgt_counter_reload();
   
  }
  /* USER CODE END 5 */
}


void vRestartNode( void )
{
	xEventGroupClearBitsFromISR(xResetEventHandle,RESTART_DISABLE);
	return;
}

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
