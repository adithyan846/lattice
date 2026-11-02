/***********************************************************************************
 * FILE ID 		: Main.c
 * VERSION NUMBER 	: 1.0.0
 * PROJECT CODE 	: ITS04T
 * PROJECT NAME 	: Test FreeRTOS- for Universal Traffic Control System CUTE Adv
 * ARCHITECT            : Hemant
 * DESIGNER             :
 * AUTHOR 	     	: Hemant
 * DATE OFLAST UPDATION : 05.11.2022
 * DESCRIPTION 		:
 ************************************************************************************/

/* FreeRTOS kernel includes. */
//#include <FreeRTOS.h>
//#include <task.h>
/* thejas64 includes. */
#include "board.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_uart.h"
#include "stdlib.h"
#include "led.h"
#include "timer.h"

////////////////////////////////////////////////////////////
/* Standard includes. */

#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

/* thejas64 incldues. */


/* Standard demo application includes. */
#include "dynamic.h"
#include "blocktim.h"
#include "GenQTest.h"
#include "recmutex.h"
#include "TimerDemo.h"
#include "EventGroupsDemo.h"
#include "TaskNotify.h"
#include "AbortDelay.h"
#include "countsem.h"
#include "death.h"

#include "encoding.h"

#include "stdlib.h"
#include "pwmc.h"

#include "config.h"




///////////////////////////////////////////////////////////
#define mainCREATE_DEMO_ONLY	0
#define mainHART_0 		0

//Prototypes for the standard FreeRTOS callback/hook  https://www.freertos.org/a00016.html
void
vApplicationMallocFailedHook (void);
void
vApplicationIdleHook (void);
void
vApplicationStackOverflowHook (TaskHandle_t pxTask, char *pcTaskName);
void
vApplicationTickHook (void);

static void
prvSetupHardware (void);	//Prepare haredware to run the demo.
/* Send a messaage to the UART initialised in prvSetupHardware. */

void
vSendString (const char *const pcString);
void
Transmit_uart (UC*);


#define mainCHECK_TASK_PRIORITY  (tskIDLE_PRIORITY+3)

static void TestTask1 (void *pvParameters);
static void TestTask2 (void *pvParameters);
static void TestTask3 (void *pvParameters);

extern void
vSendString (const char *const pcString);
unsigned char task1_stat = 0, task2_stat = 0, task3_stat = 0;
extern void
vToggleLED (void);



int
main (void)
{
  /* Bare BSP + printf blink to debug UART. No FreeRTOS, no HAL. */
  GPIO_write_pin (24, OFF_LED);

  while (1)
    {
      GPIO_write_pin (24, ON_LED);
      printf ("LED ON  1\n\r");
      timer_put_delay (TIMER_0, SYSTEM_FREQUENCY / 2);

      GPIO_write_pin (24, OFF_LED);
      printf ("LED OFF  2\n\r");
      timer_put_delay (TIMER_0, SYSTEM_FREQUENCY / 2);
    }

  for (;;)
    ;
}


static void
prvSetupHardware (void)
{
  interrupt_disable_all ();
  disable_irq ();

  vSendString ("UART [OK]\n\r");
}

/*------------------------------------------------------------*/



static void
TestTask1 (void *pvParameters)
{

     static TickType_t xNextWakeTime;

     int task1_time = 0;
     while(1)
     {

         vSendString("Task 1 !\n\r");
         vTaskDelay(pdMS_TO_TICKS(2000));
     }
      vTaskDelete( NULL );

}

static void
TestTask2 (void *pvParameters)
{

      int task2_time = 0;

      while(1)
  	vSendString("Task 2 !\n\r");
  	vTaskDelay(pdMS_TO_TICKS(2000));
      vTaskDelete( NULL );
}

static void
TestTask3 (void *pvParameters)
{

     int task3_time = 0;

     while(1)
    {
  		vSendString("Task 3 !\n\r");
  		vTaskDelay(pdMS_TO_TICKS(3000));
    }
     vTaskDelete( NULL );
}




/*-----------------------------------------------------------*/
void
vSendString (const char *const pcString)
{
  Transmit_uart ((char*) pcString);
}

/*-----------------------------------------------------------*/

void
vApplicationMallocFailedHook (void)
{
  /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created.  It is also called by the parts of the
   demo application.  If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information about how the remaining heap might be fragmented). */
  vSendString ("@ ApplicationMallocFailedHook\n\r");
  taskDISABLE_INTERRUPTS();
  __asm volatile( "ebreak" );
  for (;;)
    ;
}
/*-----------------------------------------------------------*/

void
vApplicationIdleHook (void)
{
  //vSendString("\r @ vApplicationIdleHook\n\r");
  /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
   to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
   task.  It is essential that code added to this hook function never attempts
   to block in any way (for example, call xQueueReceive() with a block time
   specified, or call vTaskDelete() with a block time).  If the application
   makes use of the vTaskDelete() API function then it is also important
   that vApplicationIdleHook() is permitted to return to the calling
   function, because it is the responsibility of the idle task to clean up
   memory allocated by the kernel to any task that has since been deleted. */
}
/*-----------------------------------------------------------*/

void
vApplicationStackOverflowHook (TaskHandle_t pxTask, char *pcTaskName)
{
  (void) pcTaskName;
  (void) pxTask;

  printf ("stack overflow %x %s\r\n", pxTask, (portCHAR*) pcTaskName);
  vSendString ("@ vApplicationStackOverflowHook\n\r");
  //printf("taskname =%s\n\r",pcTaskName);
  /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  It is called
   if a stack overflow is detected. */
  taskDISABLE_INTERRUPTS();
  //  __asm volatile( "ebreak" );
  for (;;)
    ;
}
/*-----------------------------------------------------------*/

void
vApplicationTickHook (void)
{
  vSendString ("@ vApplicationTickHook\n\r");
  /* The tests in the full demo expect some interaction with interrupts. */
}


