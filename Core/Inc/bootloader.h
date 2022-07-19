#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_


#include "main.h"


extern CRC_HandleTypeDef hcrc;
extern UART_HandleTypeDef huart2;


/*Define constant values*/
/*----------------------------------------------Defines-----------------------------------------------------------*/
#define BL_VERSION 1.0 //bootloader version
#define FRAME_0_SIZE 11 //the leng of the frame 0
#define READ_CMD 0x52
#define WRITE_CMD 0x57
#define BL_ACK 0x41    //'A'
#define BL_NACK 0x4E   //'N'
#define APP_A 0x8010000  //address for default app - A
#define APP_B 0x8020000 //address for OTA app - B

/*--------------------------------------FUNCTION PROTOTYPE------------------------------------------------*/

/*Other variables
 * writeBuffer: for storing data for OTA updating before call the flash_mem_Write function.
 * */
extern uint8_t temp[4];
extern char usr_msg[250];

//Buffer for maximum 32kB firmware
extern uint8_t writeBuffer[FLASH_PAGE_SIZE*16];


/**************** STRUCTURE DEFINITION *****************/
/*Structure mainly for storing bootloader_len and bootloader_addr
 * send through QUEUE in a more compact way. */
struct AMessage{
	uint16_t bootloader_len;
	uint32_t bootloader_addr;
};

/*User defined function
 * bootloader_verify_crc: function for generate CRC32
 * @*pData: pointer to Data array
 * @len: len of the data for CRC32
 */
int32_t bootloader_verify_crc (uint8_t *pData, uint32_t len);


/* byte_to_word: combine 4 bytes to one single word.*/
uint32_t byte_to_word(uint8_t *pData);


/* word_to_4bytes: detach one single word -> 4 bytes.*/
void word_to_4bytes(uint32_t pData);


/* bootloader_mem_write: bootloader function for writing data into Flash memory.
 * @bootloader_addr: starting address for data writing.
 * @bootloader_len: the len of data for OTA updating.
 * */
void bootloader_mem_write(uint32_t bootloader_addr, uint16_t bootloader_len);


/** bootloader_jump_application: bootloader function for jumping to application in Flash memory
 * @bootloader_addr: Addr of the application*/
void bootloader_jump_application(uint32_t bootloader_addr);

/*send ack back to host*/
void bootloader_send_ack(void);

/*send nack back to host*/
void bootloader_send_nack(void);




#endif /* INC_BOOTLOADER_H_ */
