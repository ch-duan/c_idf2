/*
 * stFlash.c
 *
 *  Created on: 2021年12月21日
 *      Author: Administrator
 */

#include "stFlash.h"

#include <stdio.h>

#include "cmsis_os2.h"

#if defined(STM32F405xx) || defined(STM32F415xx) || defined(STM32F407xx) || defined(STM32F417xx)
const uint32_t flash_sector_start_addr[] = {ADDR_FLASH_SECTOR_0, ADDR_FLASH_SECTOR_1, ADDR_FLASH_SECTOR_2,  ADDR_FLASH_SECTOR_3,
                                            ADDR_FLASH_SECTOR_4, ADDR_FLASH_SECTOR_5, ADDR_FLASH_SECTOR_6,  ADDR_FLASH_SECTOR_7,
                                            ADDR_FLASH_SECTOR_8, ADDR_FLASH_SECTOR_9, ADDR_FLASH_SECTOR_10, ADDR_FLASH_SECTOR_11};
#elif defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx) || defined(STM32F439xx)

const uint32_t flash_sector_start_addr[] = {ADDR_FLASH_SECTOR_0,  ADDR_FLASH_SECTOR_1,  ADDR_FLASH_SECTOR_2,  ADDR_FLASH_SECTOR_3,  ADDR_FLASH_SECTOR_4,
                                            ADDR_FLASH_SECTOR_5,  ADDR_FLASH_SECTOR_6,  ADDR_FLASH_SECTOR_7,  ADDR_FLASH_SECTOR_8,  ADDR_FLASH_SECTOR_9,
                                            ADDR_FLASH_SECTOR_10, ADDR_FLASH_SECTOR_11, ADDR_FLASH_SECTOR_12, ADDR_FLASH_SECTOR_13, ADDR_FLASH_SECTOR_14,
                                            ADDR_FLASH_SECTOR_15, ADDR_FLASH_SECTOR_16, ADDR_FLASH_SECTOR_17, ADDR_FLASH_SECTOR_18, ADDR_FLASH_SECTOR_19,
                                            ADDR_FLASH_SECTOR_20, ADDR_FLASH_SECTOR_21, ADDR_FLASH_SECTOR_22, ADDR_FLASH_SECTOR_23};
#endif

void jump2APP(uint32_t addr) {
  uint32_t JumpAddress;
  pFunction Jump_To_Application;

  printf("BOOTLOADER Start start addr: 0x%x\r\n", addr);
  // Check

  uint32_t stack_point = ((*(__IO uint32_t *) addr) & 0x2FFE0000);

  if (stack_point >= STACK_START && stack_point <= STACK_END + 1) {
    printf("APP Start...\r\n");
    // Jump to user application //
    HAL_SuspendTick();  // SysTick shutdown
    HAL_RCC_DeInit();
    HAL_DeInit();  // Periphery DeInit
    JumpAddress = *(__IO uint32_t *) (addr + 4);
    Jump_To_Application = (pFunction) JumpAddress;
    //    __set_PRIMASK(1);
    __disable_irq();
    // Initialize user application's Stack Pointer //
    Jump_To_Application();
  } else {
    printf("No APP found!!!\r\n");
  }
}

void FLASH_If_Init(void) {
  /* Unlock the Program memory */
  HAL_FLASH_Unlock();
  /* Clear all FLASH flags */
#ifdef STMF1
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR | FLASH_FLAG_OPTVERR);
#endif
#ifdef STM32F407xx
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
#endif
  /* Unlock the Program memory */
}

uint32_t FLASH_If_Erase_sector(uint32_t sector) {
  FLASH_EraseInitTypeDef pEraseInit;
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t SectorError;
  /* Unlock the Flash to enable the flash control register access *************/
  FLASH_If_Init();
  /* Get the sector where start the user flash area */
  printf("Erase flash Sector:%u\r\n", sector);
  pEraseInit.TypeErase = TYPEERASE_SECTORS;
  pEraseInit.Sector = sector;
  pEraseInit.NbSectors = 1;
  pEraseInit.VoltageRange = VOLTAGE_RANGE_3;
  status = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
  HAL_FLASH_Lock();
  return status;
}

uint32_t FLASH_If_Erase(uint32_t StartAddr, uint32_t EndAddr) {
  HAL_StatusTypeDef status = HAL_ERROR;
#if defined(STM32F1)
  FLASH_EraseInitTypeDef pEraseInit;
  uint32_t PageError = 0;
  HAL_FLASH_Unlock();
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR | FLASH_FLAG_OPTVERR);
  if (StartAddr < 0x08040000 && EndAddr < 0x08040000) {
    pEraseInit.Banks = FLASH_BANK_1;
    pEraseInit.Page = (StartAddr - FLASH_BASE) / FLASH_PAGE_SIZE;
    pEraseInit.NbPages = (EndAddr - FLASH_BASE) / FLASH_PAGE_SIZE - pEraseInit.Page;
    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
    printf("Flash erase page start1=%lu,end=%lu,bank=%lu,%#lx,%#lx,status:%d\r\n", pEraseInit.Page, pEraseInit.NbPages, pEraseInit.Banks, StartAddr, EndAddr,
           status);
  } else if (StartAddr < 0x08040000 && EndAddr > 0x08040000) {
    pEraseInit.Banks = FLASH_BANK_1;
    pEraseInit.Page = (StartAddr - FLASH_BASE) / FLASH_PAGE_SIZE;
    pEraseInit.NbPages = (0x0803FFFF - FLASH_BASE) / FLASH_PAGE_SIZE - pEraseInit.Page;
    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
    printf("Flash erase page start2=%lu,end=%lu,bank=%lu,%#lx,%#lx,status:%d\r\n", pEraseInit.Page, pEraseInit.NbPages, pEraseInit.Banks, StartAddr, EndAddr,
           status);
    pEraseInit.Banks = FLASH_BANK_2;
    pEraseInit.Page = (0x08040000 - FLASH_BASE) / FLASH_PAGE_SIZE + 128;
    pEraseInit.NbPages = (EndAddr - FLASH_BASE) / FLASH_PAGE_SIZE - pEraseInit.Page + 128;
    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
    printf("Flash erase page start3=%lu,end=%lu,bank=%lu,%#lx,%#lx,status:%d\r\n", pEraseInit.Page, pEraseInit.NbPages, pEraseInit.Banks, StartAddr, EndAddr,
           status);
  } else if (StartAddr >= 0x08040000) {
    pEraseInit.Banks = FLASH_BANK_2;
    pEraseInit.Page = (StartAddr - FLASH_BASE) / FLASH_PAGE_SIZE + 128;
    pEraseInit.NbPages = (EndAddr - FLASH_BASE) / FLASH_PAGE_SIZE - pEraseInit.Page + 128;
    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
    printf("Flash erase page start4=%lu,end=%lu,bank=%lu,%#lx,%#lx,status:%d\r\n", pEraseInit.Page, pEraseInit.NbPages, pEraseInit.Banks, StartAddr, EndAddr,
           status);
  }
  HAL_FLASH_Lock();
  return status;
#endif
#if defined(STM32F4)
  uint32_t startSector;
  uint32_t endSector;
  uint32_t SectorError;
  FLASH_EraseInitTypeDef pEraseInit;

  /* Unlock the Flash to enable the flash control register access *************/
  FLASH_If_Init();
  /* Get the sector where start the user flash area */
  startSector = GetSector(StartAddr);
  endSector = GetSector(EndAddr);
  printf("Erase flash startSector:%u,endSector:%u\r\n", startSector, endSector);
  pEraseInit.TypeErase = TYPEERASE_SECTORS;
  pEraseInit.Sector = startSector;
  pEraseInit.NbSectors = (endSector - startSector) == 0 ? 1 : (endSector - startSector) + 1;
  pEraseInit.VoltageRange = VOLTAGE_RANGE_3;
  status = HAL_FLASHEx_Erase(&pEraseInit, &SectorError);
  HAL_FLASH_Lock();
#endif
  return status;
}

HAL_StatusTypeDef FlashWriteEnv(uint32_t addr, uint32_t *p_source, uint32_t len) {
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t i;
  printf("len:%u\r\n", len);
#if defined(STM32F1)
  while (FLASH_If_Erase(addr, addr + (len / 0x800 + 1) * 0x800) != 0) {
  }
#endif
#if defined(STM32F4)
  while (FLASH_If_Erase(addr, addr + len) != 0) {
  }
#endif
  // unlock
  FLASH_If_Init();
  __HAL_FLASH_DATA_CACHE_DISABLE();
  __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

  __HAL_FLASH_DATA_CACHE_RESET();
  __HAL_FLASH_INSTRUCTION_CACHE_RESET();

  __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  __HAL_FLASH_DATA_CACHE_ENABLE();
  /* DataLength must be a multiple of 64 bit */
  for (i = 0; i <= len; i += 4) {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
     be done by word */
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, *p_source);
    if (status == HAL_OK) {
      /* Check the written value */
      if (*(uint32_t *) addr != *p_source) {
        /* Flash content doesn't match SRAM content */
        printf("flash env check error %x,%u,%u\r\n", addr, status, i);
        break;
      }
      /* Increment FLASH destination address */
    } else {
      /* Error occurred while writing data in Flash memory */
      printf("flash env write error %x,%u,%u\r\n", addr, status, i);
      break;
    }
    p_source++;
    addr += 4;
  }
  HAL_FLASH_Lock();
  if (status != HAL_OK) {
    printf("flashWriteEnv check error %x,%d\r\n", addr, status);
  }
  return status;
}

uint32_t FLASH_If_Write(uint32_t destination, uint32_t endAddr, uint8_t *buffer, uint32_t length) {
  uint32_t status = HAL_ERROR;
  uint32_t *p_source = (uint32_t *) buffer;
  uint32_t i = 0;
  uint8_t err_cnt = 0;
  /* Unlock the Flash to enable the flash control register access *************/
  FLASH_If_Init();
  __HAL_FLASH_DATA_CACHE_DISABLE();
  __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();

  __HAL_FLASH_DATA_CACHE_RESET();
  __HAL_FLASH_INSTRUCTION_CACHE_RESET();

  __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
  __HAL_FLASH_DATA_CACHE_ENABLE();
  /* DataLength must be a multiple of 64 bit */
  for (i = 0; (i < length / 4) && (destination <= (endAddr - 4));) {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
     be done by word */
    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destination, p_source[i]);
    if (status == HAL_OK) {
      /* Check the written value */
      if (*(uint32_t *) destination != *(p_source + i)) {
        /* Flash content doesn't match SRAM content */
        if (++err_cnt >= 5) {
          printf("flash check error %X,%u,%u. Src:%X,Des:%X\r\n", destination, status, i, *(p_source + i), *(uint32_t *) destination);
          printf("Exit because retry to write flash failed more than 5 times.\r\n");
          HAL_FLASH_Lock();
          return -1;
        }
        continue;
      }
      err_cnt = 0;
      /* Increment FLASH destination address */
      i++;
      destination += 4;
    } else {
      /* Error occurred while writing data in Flash memory */
      printf("flash write error %x,%u,%u\r\n", destination, status, i);
      break;
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
   to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  return status;
}

int FLSH_If_Erase_Write_Sector(int sector, uint8_t *data, int size) {
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t addr = GetSectorStartAddr(sector);
  uint32_t statusWrite = 0, statusErase = 0;
  statusErase = FLASH_If_Erase_sector(sector);
  printf("Erase status:%u\r\n", statusErase);
  if (HAL_OK != status) {
    return -1;
  }
  statusWrite = FLASH_If_Write(addr, addr + size, data, size);
  printf("write status:%u\r\n", statusErase);
  if (HAL_OK != statusWrite) {
    return -1;
  }
  return 0;
}

/**
 * @brief  Gets the sector of a given address
 * @param  Address: Flash address
 * @retval The sector of a given address
 */

uint32_t GetSectorStartAddr(uint32_t sector) {
  uint32_t addr = 0;
  if (sector <= 23) {
    addr = flash_sector_start_addr[sector];
  } else {
    addr = -1;
  }
  return addr;
}

uint32_t GetSectorSize(uint32_t sector) {
  uint32_t size = 0;
  if ((sector <= 3) || (sector >= 12 && sector <= 15)) {
    size = 0x4000;  // 16K
  } else if (sector == 4 || sector == 16) {
    size = 0x10000;  // 64K
  } else if ((sector >= 5 && sector <= 11) || (sector >= 17 && sector <= 23)) {
    size = 0x20000;  // 128K
  } else {
    size = 0;
  }
  return size;
}

uint32_t GetSector(uint32_t Address) {
  uint32_t sector = 0;
#if defined(STM32F405xx) || defined(STM32F415xx) || defined(STM32F407xx) || defined(STM32F417xx)
  if ((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0)) {
    sector = FLASH_SECTOR_0;
  } else if ((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1)) {
    sector = FLASH_SECTOR_1;
  } else if ((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2)) {
    sector = FLASH_SECTOR_2;
  } else if ((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3)) {
    sector = FLASH_SECTOR_3;
  } else if ((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4)) {
    sector = FLASH_SECTOR_4;
  } else if ((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5)) {
    sector = FLASH_SECTOR_5;
  } else if ((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6)) {
    sector = FLASH_SECTOR_6;
  } else if ((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7)) {
    sector = FLASH_SECTOR_7;
  } else if ((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8)) {
    sector = FLASH_SECTOR_8;
  } else if ((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9)) {
    sector = FLASH_SECTOR_9;
  } else if ((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10)) {
    sector = FLASH_SECTOR_10;
  } else {
    sector = FLASH_SECTOR_11;
  }
#endif
#if defined(STM32F427xx) || defined(STM32F437xx) || defined(STM32F429xx) || defined(STM32F439xx)
  if ((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11)) {
    sector = FLASH_SECTOR_11;
  } else if ((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12)) {
    sector = FLASH_SECTOR_12;
  } else if ((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13)) {
    sector = FLASH_SECTOR_13;
  } else if ((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14)) {
    sector = FLASH_SECTOR_14;
  } else if ((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15)) {
    sector = FLASH_SECTOR_15;
  } else if ((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16)) {
    sector = FLASH_SECTOR_16;
  } else if ((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17)) {
    sector = FLASH_SECTOR_17;
  } else if ((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18)) {
    sector = FLASH_SECTOR_18;
  } else if ((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19)) {
    sector = FLASH_SECTOR_19;
  } else if ((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20)) {
    sector = FLASH_SECTOR_20;
  } else if ((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21)) {
    sector = FLASH_SECTOR_21;
  } else if ((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22)) {
    sector = FLASH_SECTOR_22;
  } else /*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23))*/
  {
    sector = FLASH_SECTOR_23;
  }
#endif
  return sector;
}

int internal_flash_write(uint32_t addr, uint8_t *in_data, uint32_t size) {
  __disable_irq();
  osKernelLock();
  uint32_t sector = GetSector(addr);
  HAL_StatusTypeDef status = HAL_OK;
  uint32_t statusWrite = 0, statusErase = 0;
  statusErase = FLASH_If_Erase_sector(sector);
  printf("Erase status:%u\r\n", statusErase);
  if (HAL_OK != status) {
    return -1;
  }
  statusWrite = FLASH_If_Write(addr, addr + size, in_data, size);
  printf("write status:%u\r\n", statusErase);
  if (HAL_OK != statusWrite) {
    return -1;
  }
  osKernelUnlock();
  __enable_irq();
  return 0;
}

int internal_flash_read(uint32_t addr, uint32_t *out_data, uint32_t size) {
  for (uint32_t i = addr; i < addr + size; i++) {
    *(out_data + i) = *(__IO uint32_t *) i;
  }
  return 0;
}