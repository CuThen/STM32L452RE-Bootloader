#ifndef INC_CUSTOM_RTOS_H_
#define INC_CUSTOM_RTOS_H_

#include "cmsis_os.h"
/*Declare Task prototypes	uint16_t bootloader_len;
 * vTaskOTACheck: task for checking if user want to OTA or not if not just jump to default app
 * vTaskCMDRx: Receive the CMD + Addr from the first frame ;first frame: |W|ADDR|LEN|CRC32|
 * vTaskWriteHandler: Task for handling Write to Flash memory command
 * */
void vTaskOTACheck(void * pvParameters);
void vTaskCMDRx(void * pvParameters);
void vTaskCMDHandler(void * pvParameters);
void vTaskWriteHandler(void *pvParameters);

/*Declare timer call back function*/
void vTimerCallBack( TimerHandle_t xTimer);


/*Declare Task handle
 * TaskHandle1: OTA CMD receive handler
 * TaskHandle2: CMD receive handler
 * TaskHandle3: CMD processing handler
 * */
extern TaskHandle_t xTaskHandle1;
extern TaskHandle_t xTaskHandle2;
extern TaskHandle_t xTaskHandle3;
extern TaskHandle_t xTaskHandle4;

/*Declare Queue handle
 * */
extern QueueHandle_t xFrame_0;
extern QueueHandle_t xStructQueue;

/*Declare Timer Handle
 * Timer_1
 * */
extern TimerHandle_t xTimer_1;


#endif /* INC_CUSTOM_RTOS_H_ */
