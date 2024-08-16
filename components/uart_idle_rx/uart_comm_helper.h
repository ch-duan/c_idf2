#ifndef H_C_IDF_UART_COMM_HELPER_H
#define H_C_IDF_UART_COMM_HELPER_H
#include "stm32f4xx_hal.h"
// dma receive buf queue
#include <stdint.h>
#define RECV_BUF_MAX        8192
#define USE_RXEVENTCALLBACK 1
typedef void (*PacketArrived)(uint8_t *packet, uint16_t packetLen, void *arg);

typedef struct {
  UART_HandleTypeDef *uart;
  PacketArrived deliver_packet;
  void *arg;

  uint16_t FrameLength;
  uint8_t *m_framePayload;
  uint32_t m_recv_packet_max;
} LinkLayerHandler;

typedef struct {
  void *priv;
  LinkLayerHandler llhandler;
} UartMessageHandler;

#ifdef __cplusplus
extern "C" {
#endif
int LLInit(LinkLayerHandler *self, UART_HandleTypeDef *uart, uint8_t *recv_buffer, uint16_t recv_packet_max);
void LLSendData(LinkLayerHandler *self, uint8_t *data, size_t len);
void UART_IDLE_Callback(LinkLayerHandler *self, UART_HandleTypeDef *huart);

int UartPacketHandlerInit(UartMessageHandler *msgHandler, UART_HandleTypeDef *uart, uint8_t *recv_buffer, uint16_t recv_packet_max,
                          PacketArrived pHandlerPacket);
void reset_uart_error(UartMessageHandler *msgHandler);
#ifdef __cplusplus
}
#endif

#endif