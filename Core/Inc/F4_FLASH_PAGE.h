#ifndef INC_F4_FLASH_PAGE_H_
#define INC_F4_FLASH_PAGE_H_

#include "stm32l4xx_hal.h"
/**
 * @brief  Read n bytes from Flash memory --> save to RxBuf variable
 * @param  StartPageAddress: The address you want to read from
 * @param  RxBuf: storing buffer data
 * @param  numberofbytes: number of bytes you want to read
 */
void flash_Read_Data (uint32_t StartPageAddress, uint8_t *RxBuf, uint16_t numberofbytes);

/*Write n double words from Flash memory from Data variable */
uint32_t flash_Write_Data (uint32_t StartPageAddress, uint64_t *Data, uint16_t numberof2words);

#endif /* INC_F4_FLASH_PAGE_H_ */
