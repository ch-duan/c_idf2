

#include <stdarg.h>

#include "cmsis_os2.h"
#include "driver_w25qxx_interface.h"
#include "dwt_stm32_delay.h"
#include "spi.h"

/**
 * @brief     spi bus write command
 * @param[in] *buf points to a data buffer
 * @param[in] len is the length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t spi_write_cmd(uint8_t *buf, uint16_t len) {
  uint8_t res;

  /* set cs low */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_RESET);

  /* if len > 0 */
  if (len > 0) {
    /* transmit the buffer */
    res = HAL_SPI_Transmit(&hspi3, buf, len, 1000);
    if (res != HAL_OK) {
      /* set cs high */
      HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

      return 1;
    }
  }

  /* set cs high */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

  return 0;
}

/**
 * @brief     spi bus write
 * @param[in] addr is the spi register address
 * @param[in] *buf points to a data buffer
 * @param[in] len is the length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t spi_write(uint8_t addr, uint8_t *buf, uint16_t len) {
  uint8_t buffer;
  uint8_t res;

  /* set cs low */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_RESET);

  /* transmit the addr */
  buffer = addr;
  res = HAL_SPI_Transmit(&hspi3, (uint8_t *) &buffer, 1, 1000);
  if (res != HAL_OK) {
    /* set cs high */
    HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

    return 1;
  }

  /* if len > 0 */
  if (len > 0) {
    /* transmit the buffer */
    res = HAL_SPI_Transmit(&hspi3, buf, len, 1000);
    if (res != HAL_OK) {
      /* set cs high */
      HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

      return 1;
    }
  }

  /* set cs high */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

  return 0;
}

/**
 * @brief     spi bus write address 16
 * @param[in] addr is the spi register address
 * @param[in] *buf points to a data buffer
 * @param[in] len is the length of the data buffer
 * @return    status code
 *            - 0 success
 *            - 1 write failed
 * @note      none
 */
uint8_t spi_write_address16(uint16_t addr, uint8_t *buf, uint16_t len) {
  uint8_t buffer[2];
  uint8_t res;

  /* set cs low */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_RESET);

  /* transmit the addr  */
  buffer[0] = (addr >> 8) & 0xFF;
  buffer[1] = addr & 0xFF;
  res = HAL_SPI_Transmit(&hspi3, (uint8_t *) buffer, 2, 1000);
  if (res != HAL_OK) {
    /* set cs high */
    HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

    return 1;
  }

  /* if len > 0 */
  if (len > 0) {
    /* transmit the buffer */
    res = HAL_SPI_Transmit(&hspi3, buf, len, 1000);
    if (res != HAL_OK) {
      /* set cs high */
      HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

      return 1;
    }
  }

  /* set cs high */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

  return 0;
}

/**
 * @brief      spi bus read command
 * @param[out] *buf points to a data buffer
 * @param[in]  len is the length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t spi_read_cmd(uint8_t *buf, uint16_t len) {
  uint8_t res;

  /* set cs low */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_RESET);

  /* if len > 0 */
  if (len > 0) {
    /* receive to the buffer */
    res = HAL_SPI_Receive(&hspi3, buf, len, 1000);
    if (res != HAL_OK) {
      /* set cs high */
      HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

      return 1;
    }
  }

  /* set cs high */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

  return 0;
}

/**
 * @brief      spi bus read
 * @param[in]  addr is the spi register address
 * @param[out] *buf points to a data buffer
 * @param[in]  len is the length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t spi_read(uint8_t addr, uint8_t *buf, uint16_t len) {
  uint8_t buffer;
  uint8_t res;

  /* set cs low */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_RESET);

  /* transmit the addr */
  buffer = addr;
  res = HAL_SPI_Transmit(&hspi3, (uint8_t *) &buffer, 1, 1000);
  if (res != HAL_OK) {
    /* set cs high */
    HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

    return 1;
  }

  /* if len > 0 */
  if (len > 0) {
    /* receive to the buffer */
    res = HAL_SPI_Receive(&hspi3, buf, len, 1000);
    if (res != HAL_OK) {
      /* set cs high */
      HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

      return 1;
    }
  }

  /* set cs high */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

  return 0;
}

/**
 * @brief      spi bus read address 16
 * @param[in]  addr is the spi register address
 * @param[out] *buf points to a data buffer
 * @param[in]  len is the length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 read failed
 * @note       none
 */
uint8_t spi_read_address16(uint16_t addr, uint8_t *buf, uint16_t len) {
  uint8_t buffer[2];
  uint8_t res;

  /* set cs low */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_RESET);

  /* transmit the addr  */
  buffer[0] = (addr >> 8) & 0xFF;
  buffer[1] = addr & 0xFF;
  res = HAL_SPI_Transmit(&hspi3, (uint8_t *) buffer, 2, 1000);
  if (res != HAL_OK) {
    /* set cs high */
    HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

    return 1;
  }

  /* if len > 0 */
  if (len > 0) {
    /* receive to the buffer */
    res = HAL_SPI_Receive(&hspi3, buf, len, 1000);
    if (res != HAL_OK) {
      /* set cs high */
      HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

      return 1;
    }
  }

  /* set cs high */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

  return 0;
}

/**
 * @brief      spi transmit
 * @param[in]  *tx points to a tx buffer
 * @param[out] *rx points to a rx buffer
 * @param[in]  len is the length of the data buffer
 * @return     status code
 *             - 0 success
 *             - 1 transmit failed
 * @note       none
 */
uint8_t spi_transmit(uint8_t *tx, uint8_t *rx, uint16_t len) {
  uint8_t res;

  /* set cs low */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_RESET);

  /* if len > 0 */
  if (len > 0) {
    /* transmit */
    res = HAL_SPI_TransmitReceive(&hspi3, tx, rx, len, 1000);
    if (res != HAL_OK) {
      /* set cs high */
      HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

      return 1;
    }
  }

  /* set cs high */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

  return 0;
}

/**
 * @brief      spi bus write read
 * @param[in]  *in_buf points to an input buffer
 * @param[in]  in_len is the input length
 * @param[out] *out_buf points to an output buffer
 * @param[in]  out_len is the output length
 * @return     status code
 *             - 0 success
 *             - 1 write read failed
 * @note       none
 */
uint8_t spi_write_read(uint8_t *in_buf, uint32_t in_len, uint8_t *out_buf, uint32_t out_len) {
  uint8_t res;

  /* set cs low */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_RESET);

  /* if in_len > 0 */
  if (in_len > 0) {
    /* transmit the input buffer */
    res = HAL_SPI_Transmit(&hspi3, in_buf, in_len, 1000);
    if (res != HAL_OK) {
      /* set cs high */
      HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

      return 1;
    }
  }

  /* if out_len > 0 */
  if (out_len > 0) {
    /* transmit to the output buffer */
    res = HAL_SPI_Receive(&hspi3, out_buf, out_len, 1000);
    if (res != HAL_OK) {
      /* set cs high */
      HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

      return 1;
    }
  }

  /* set cs high */
  HAL_GPIO_WritePin(w25_NSS_GPIO_Port, w25_NSS_Pin, GPIO_PIN_SET);

  return 0;
}

/**
 * @brief  interface spi qspi bus init
 * @return status code
 *         - 0 success
 *         - 1 spi qspi init failed
 * @note   none
 */
uint8_t w25qxx_interface_spi_qspi_init(void) {
  // MX_SPI3_Init();
  return 0;
}

/**
 * @brief  interface spi qspi bus deinit
 * @return status code
 *         - 0 success
 *         - 1 spi qspi deinit failed
 * @note   none
 */
uint8_t w25qxx_interface_spi_qspi_deinit(void) {
  /* spi deinit */
  // if (HAL_SPI_DeInit(&hspi3) != HAL_OK) {
  //   return 1;
  // }
  return 0;
}

/**
 * @brief      interface spi qspi bus write read
 * @param[in]  instruction is the sent instruction
 * @param[in]  instruction_line is the instruction phy lines
 * @param[in]  address is the register address
 * @param[in]  address_line is the address phy lines
 * @param[in]  address_len is the address length
 * @param[in]  alternate is the register address
 * @param[in]  alternate_line is the alternate phy lines
 * @param[in]  alternate_len is the alternate length
 * @param[in]  dummy is the dummy cycle
 * @param[in]  *in_buf points to a input buffer
 * @param[in]  in_len is the input length
 * @param[out] *out_buf points to a output buffer
 * @param[in]  out_len is the output length
 * @param[in]  data_line is the data phy lines
 * @return     status code
 *             - 0 success
 *             - 1 write read failed
 * @note       none
 */
uint8_t w25qxx_interface_spi_qspi_write_read(uint8_t instruction, uint8_t instruction_line, uint32_t address, uint8_t address_line, uint8_t address_len,
                                             uint32_t alternate, uint8_t alternate_line, uint8_t alternate_len, uint8_t dummy, uint8_t *in_buf, uint32_t in_len,
                                             uint8_t *out_buf, uint32_t out_len, uint8_t data_line) {
  if ((instruction_line != 0) || (address_line != 0) || (alternate_line != 0) || (dummy != 0) || (data_line != 1)) {
    return 1;
  }

  return spi_write_read(in_buf, in_len, out_buf, out_len);
}

/**
 * @brief     interface delay ms
 * @param[in] ms
 * @note      none
 */
void w25qxx_interface_delay_ms(uint32_t ms) {
  osDelay(ms);
}

/**
 * @brief     interface delay us
 * @param[in] us
 * @note      none
 */
void w25qxx_interface_delay_us(uint32_t us) {
  DWT_Delay_us(us);
}


/**
 * @brief     interface print format data
 * @param[in] fmt is the format data
 * @note      none
 */
void w25qxx_interface_debug_print(const char *const fmt, ...)
{
    char str[256];
    uint16_t len;
    va_list args;
    
    memset((char *)str, 0, sizeof(char) * 256); 
    va_start(args, fmt);
    vsnprintf((char *)str, 255, (char const *)fmt, args);
    va_end(args);
    
    len = strlen((char *)str);
    (void)printf((char *)str, len);
}
