#include "custom_rtos.h"
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bootloader.h"
#include "F4_FLASH_PAGE.h"


/*Define TaskHandle*/
TaskHandle_t xTaskHandle1 = NULL;
TaskHandle_t xTaskHandle2 = NULL;
TaskHandle_t xTaskHandle3 = NULL;
TaskHandle_t xTaskHandle4 = NULL;

/*Define QueueHandle
 * xFrame_0: Queue for sending the Frame_0 queue*/
QueueHandle_t xFrame_0 = NULL;
QueueHandle_t xStructQueue;
/*Declare Timer Handle
 * Timer_1
 * */
TimerHandle_t xTimer_1;



/*vTaskCMDHandler: Handling the CMD from host, call the Write to Flash memory function.
 *
 * */
void vTaskCMDHandler(void * pvParameters)
{
	uint8_t bootloaderProcessBuffer[FRAME_0_SIZE];
	uint32_t crcHost = 0;
	uint8_t flag;
	struct AMessage xProcessStructure;
	while(1)
	{
		if( xQueueReceive( xFrame_0, ( void * )&(bootloaderProcessBuffer), ( TickType_t ) 10 ) == pdPASS )
		{
			printf("I have receive CMD, ready to process the data...\r\n");

	    	//get the crc32 from host combine 4 byte to get the CRC32
	    	crcHost = byte_to_word(&bootloaderProcessBuffer[FRAME_0_SIZE -1]);

			//Extract all the needed infomation to be send in the structure queue.
			xProcessStructure.bootloader_len = bootloaderProcessBuffer[6];
			xProcessStructure.bootloader_len = xProcessStructure.bootloader_len << 8;
			xProcessStructure.bootloader_len = xProcessStructure.bootloader_len | bootloaderProcessBuffer[5];

			xProcessStructure.bootloader_addr = byte_to_word(&bootloaderProcessBuffer[FRAME_0_SIZE - 6 - 1]);

	    	//flag is set when CRC check is match with the command
	    	if(crcHost ==  bootloader_verify_crc(bootloaderProcessBuffer, FRAME_0_SIZE - 4))
	    		flag = true;
	    	else flag = false;

	    	if(flag)
	    	{
	    		switch(bootloaderProcessBuffer[0])
	    		{
	    		case READ_CMD://read cmd

	    			printf("Read from Flash command\r\n");

//	    			//get the address of data want to read or write minus 4 bytes crc and 2 bytes len
//
//	    			xStatus = xQueueSend(xStructQueue, (void *) &xProcessStructure,( TickType_t ) 10);
//
//					if( xStatus != pdPASS )
//					{
//						printf("Failed to process the CMD!\r\n");
//					}
//					taskYIELD();
//	    	    	/*Call function */

	    		break;

	    		case WRITE_CMD: //write cmd

	    			printf("Starting Write to Flash process...\r\n");

	    			//Send ACK
	    			bootloader_send_ack();
	    			/*Call function */
	    			bootloader_mem_write(xProcessStructure.bootloader_addr, xProcessStructure.bootloader_len);

	    			bootloader_jump_application(APP_B);
	    		break;

	    		default:

	    			printf("Error wrong command!!\r\n");
	    		break;
	    		}
	    	}

	    	else
	    		{
	    			printf("Error CRC32 not match!! HALT \r\n");
	    			bootloader_send_nack();
	    			Error_Handler();
	    		}
		}
	}
}



/*vTaskCMDrx: Receiving the first frame of OTA: || R/W | ADDR | CRC32 ||
 *
 * */
void vTaskCMDRx(void * pvParameters)
{
	uint8_t bootloaderRxBuffer[FRAME_0_SIZE];
	portBASE_TYPE xStatus;
	while(1)
	{
		if(xTaskNotifyWait(0, 0, NULL, portMAX_DELAY) == pdTRUE)
		{

			/*Notification receive*/
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

		    memset(bootloaderRxBuffer, 0 , sizeof(bootloaderRxBuffer)); //reset the bl_rx_buffer to zero

		    printf("Please send the command |W|ADDR|LEN|:\r\n");
		    HAL_UART_Receive(&huart2, bootloaderRxBuffer, FRAME_0_SIZE , HAL_MAX_DELAY);

		    xStatus = xQueueSend(xFrame_0, ( void * )&bootloaderRxBuffer, ( TickType_t ) 10 );
	        if( xStatus != pdPASS )
	        {
	            printf("Failed to send queue to CMD Processing!\r\n");
	        }

	        vTaskSuspend(NULL);
		}
	}
}


/*Task for button checking (PC13) if button is not pressed in 10 second
 * -> Jump to Slot1 - APP A, else begin FOTAing.
 * */
void vTaskOTACheck(void * pvParameters)
{
  /* Check the GPIO for 10 seconds */
  //GPIO_PinState OTA_Pin_state;
  uint32_t end_tick = HAL_GetTick() + 10000;   // from now to 10 Seconds

  uint8_t boot_cmd[7];
  printf("Send the boot command 'OTABOOT' to trigger OTA update...\r\n");

  while(1)
  {
	  HAL_UART_Receive(&huart2, boot_cmd, sizeof(boot_cmd), 10000);
	  vTaskDelay(100);
	  uint32_t current_tick = HAL_GetTick();

	  /* Check the button is pressed or not for 10 seconds */
  	  if( (boot_cmd[0] == 'O') && (boot_cmd[1] == 'T') && (boot_cmd[2] == 'A') && (boot_cmd[3] == 'B')
  			&& (boot_cmd[4] == 'O') && (boot_cmd[5] == 'O') && (boot_cmd[6] == 'T'))
  	  {
  		 printf("Starting OTA through UART...\r\n");
  		 xTaskNotify(xTaskHandle2, 0x0, eNoAction);
  		 vTaskSuspend(NULL);
  	  }
  	  else if ( current_tick > end_tick )
  	  {
  		 bootloader_jump_application(0x8010000);
  		 vTaskSuspend(NULL);
  	  }
  }
}


/*-----------------------------------------------------------------------------------------------------------------*/
