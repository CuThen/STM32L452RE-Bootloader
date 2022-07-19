#include "string.h"
#include "stdio.h"
#include "F4_FLASH_PAGE.h"
/* STM32FL452RE have 256 PAGES (Page 0 to Page 256) of 2 KB each. This makes up 512 KB Flash Memory
 * FLASH_PAGE_SIZE = 2kB = 2048 || for other STM32 chip change the 2048 value
 */

/* GetPage Function return the address of starting page as to the  */
static uint32_t GetPage(uint32_t Address)
{
  for (int indx = 0; indx < 256; indx++)
  {
	  if((Address < (0x08000000 + (FLASH_PAGE_SIZE *(indx+1))) ) && (Address >= (0x08000000 + FLASH_PAGE_SIZE*indx)))
	  {
		  return (0x08000000 + FLASH_PAGE_SIZE*indx);
	  }
  }
  return 0;
}

/* Function for Flash writing memory mode: DOUBLE_WORD arg: number of 2 words
 * StartPageAddress doesn't necessary to be the actual start address page, it could be in the middle of the page
 * */
uint32_t flash_Write_Data(uint32_t StartPageAddress, uint64_t *Data, uint16_t numberof2words)
{
	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;

	int sofar = 0;
	  /* Unlock the Flash to enable the flash control register access *************/
	   HAL_FLASH_Unlock();

	   /* Erase the user Flash area*/

	  uint32_t StartPage = GetPage(StartPageAddress);
	  uint16_t PageIndex = (StartPageAddress - 0x08000000)/FLASH_PAGE_SIZE;
	  uint32_t EndPageAdress = StartPageAddress + numberof2words*8;
	  uint32_t EndPage = GetPage(EndPageAdress);

	   /* Fill EraseInit structure */
	   EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
	   EraseInitStruct.Page = PageIndex;
	   EraseInitStruct.NbPages = ((EndPage - StartPage)/FLASH_PAGE_SIZE) +1;

	   if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	   {
	     /*Error occurred while page erase.*/
		  return HAL_FLASH_GetError ();
	   }

	   /* Program the user Flash area word by word*/

	    while (sofar < numberof2words)
	   {
	     if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, StartPageAddress, Data[sofar]) == HAL_OK)
	     {
	    	 StartPageAddress += 8;  // use StartPageAddress += 2 for half word and 8 for double word
	    	 sofar++;
	     }
	     else
	     {
	       /* Error occurred while writing data in Flash memory*/
	    	 return HAL_FLASH_GetError ();
	     }
	   }

	   /* Lock the Flash to disable the flash control register access (recommended
	      to protect the FLASH memory against possible unwanted operation) *********/
	   HAL_FLASH_Lock();

	   return 0;
}


/* Function for Flash reading memory reading each number of bytes*/
void flash_Read_Data (uint32_t StartPageAddress, uint8_t *RxBuf, uint16_t numberofbytes)
{
	while (1)
	{
		if (!(numberofbytes--)) break;
		*RxBuf = *(__IO uint32_t *)StartPageAddress;
		StartPageAddress += 1;
		RxBuf++;
	}
}

