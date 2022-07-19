#include "bootloader.h"


/*--------------------------------------------Variables----------------------------------------------------*/

uint8_t temp[4];
char usr_msg[250] = {0};
uint8_t writeBuffer[FLASH_PAGE_SIZE*16];

/*-----------------------------------------------------------------------------------------------------------------*/


void bootloader_jump_application(uint32_t bootloader_addr)
{
	//printf("Starting default app...\r\n");
	//Jump to slot 1 - default app.
	//Turn off peripherals and all interrupt flag
	HAL_RCC_DeInit();

	//Turn off all Pending Interrupt Request and System Tick
	HAL_DeInit();

	//Turn off Fault handler
	SCB->SHCSR &= ~( SCB_SHCSR_USGFAULTENA_Msk |\
	SCB_SHCSR_BUSFAULTENA_Msk | \
	SCB_SHCSR_MEMFAULTENA_Msk ) ;

    SCB->VTOR = bootloader_addr;

	HAL_CRC_DeInit(&hcrc);

	void (*app_reset_handler)(void) = (void*)(*((volatile uint32_t*) (bootloader_addr + 4U)));

	__set_CONTROL(0); // Change from PSP to MSP
	__set_MSP(*(volatile uint32_t*) bootloader_addr);

	app_reset_handler();    //call the app reset handler
}


void bootloader_mem_write(uint32_t bootloaderAddr, uint16_t bootloaderLen)
{
	uint32_t crcValue, crcHost;

	//Receive the len of bytes of data to be written into flash memory and 4 bytes crc32.
	// Can config Maximum writeBuffer[LEN] in the bootloader.h file. NOW is 32kB
	printf("Send data for write to Flash:...\n\r");
	HAL_UART_Receive(&huart2, writeBuffer,bootloaderLen + 4 , 10000);

	//Get the CRC host
	crcHost = byte_to_word(&writeBuffer[bootloaderLen + 4 - 1]);

	//Calculate and compare the crc_value with crc_host; crc_value is calculate along with bl_len = len of the data.
	crcValue = bootloader_verify_crc(&writeBuffer, bootloaderLen);

	if(crcHost == crcValue)
	{
		uint8_t remain;
		remain = bootloaderLen % 8;

		if(remain != 0 )
		{
			//Batching the remaining bytes.
			for(uint8_t i = 0; i < 8 - remain; i++)
			{
				writeBuffer[bootloaderLen + i] = 0xFF;
			}
		}

	/*Execute the flash write memory*/
	flash_Write_Data(bootloaderAddr, writeBuffer, bootloaderLen * 8);
	}

	else
		{
			printf("Error in Write Function: Failed to check the CRC32 !! HALT \r\n");
			Error_Handler();
		}

	printf("Updating process finished...\r\n");
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5,GPIO_PIN_RESET);
}


/*-----------------------------------------------------------------------------------------------------------------*/


/* This function verifies the CRC of the given in pData with len and 8-bytes array input*/
int32_t bootloader_verify_crc (uint8_t *pData, uint32_t len)
{
	uint32_t uwCRCValue = 0xff;

    for (uint32_t i = 0 ; i < len ; i++)
	{
        uint32_t i_data = pData[i];
        uwCRCValue = HAL_CRC_Accumulate(&hcrc, &i_data, 1);
	}

	 /* Reset CRC Calculation Unit */
    __HAL_CRC_DR_RESET(&hcrc);
    return uwCRCValue;
}


/*Convert 4 byte to 1 word MSB start from behind in array Ex: MSB 12 -> 11 -> 10 -> 9 LSB*/
uint32_t byte_to_word(uint8_t *pData)
{
	uint32_t value;
	value = (uint32_t) *pData << 24;
	pData--;
    value = value | (uint32_t) *pData << 16;
    pData--;
    value = value | (uint32_t) *pData << 8;
    pData--;
    value = value | *pData;
	return value;
}


/*Convert a single word (not a array) to 4 bytes then append it to temp array*/
void word_to_4bytes(uint32_t pData)
{
	temp[0] = (pData & 0xFF000000) >> 24;
	temp[1] = (pData & 0x00FF0000) >> 16;
	temp[2] = (pData & 0x0000FF00) >> 8;
	temp[3] = pData & 0x000000FF;
}


/*This function sends ACK*/
void bootloader_send_ack()
{
	 //here we send the ack value
	uint8_t ack_buf;
	ack_buf = BL_ACK;
	HAL_UART_Transmit(&huart2,&ack_buf,1,100);
}


/*This function sends NACK"*/
void bootloader_send_nack()
{
	//here we send the nack value
	uint8_t nack_buf;
	nack_buf = BL_NACK;
	HAL_UART_Transmit(&huart2,&nack_buf,1,100);
}

/*-----------------------------------------------------------------------------------------------------------------*/
/**
  * @brief Print the characters to UART (printf).
  * @retval int
  */
#ifdef __GNUC__
  /* With GCC, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the UART3 and Loop until the end of transmission */
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
  return ch;
}
