/*
 * @Author: ch
 * @Description:
 * @Date: 2021-04-08 18:57:01
 * @LastEditTime: 2023-10-23 11:58:40
 * @LastEditors: ch
 * @version:
 * @Reference:
 */
#pragma once

#include "messageQueue_config.h"

#if MQ_USE

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define QUEUE_BUF_SIZE         10240
#define MESSAGE_QUEUE_BUF_SIZE 500

#define MQ_LOG_LEVEL_E 1
#define MQ_LOG_LEVEL_W 2
#define MQ_LOG_LEVEL_D 3
#define MQ_LOG_LEVEL_I 4
#define MQ_LOG_LEVEL_V 5

#define MQ_LOG_ENABLE  1
#define MQ_LOG_LEVEL   MQ_LOG_LEVEL_E

#define mq_log(TAG, ...) \
  do {                   \
    printf("%s:", TAG);  \
    printf(__VA_ARGS__); \
    printf("\r\n");      \
  } while (0)

#define mq_log_hex(TAG, descriptioin, data, len) \
  do {                                           \
    printf("[%s:%s]", TAG, descriptioin);        \
    for (uint16_t i = 0; i < len; i++) {         \
      if (i % 50 == 0) {                         \
        printf("\r\n");                          \
      }                                          \
      printf("%02X ", data[i]);                  \
    }                                            \
    printf("\r\n");                              \
  } while (0)

typedef struct {
  uint8_t *data;
  uint16_t len;
  void *argv;
  uint8_t ready;
} QueueBuf_t;

typedef struct {
  QueueBuf_t *queuebuf;
  int readIdx;
  int writeIdx;
  bool use_fixed_buffer;
  uint16_t mq_size;
  uint32_t buffer_size;
} messageQueue_t;

typedef enum {
  mqOK = 0,
  mqError = -1,
  mqErrorOverflow = 2,        // buff size is to long
  mqStatusBuffFull = 3,       // buff is full
  mqErrorNotEnoughSpace = 4,  // memery allocation failed
  mqStatusEmpty,              // mq is empty
  mqStatusNotEmpty,           // mq is not empyt
  mqStatusNotInit,            // mq is not init
  mqStatusNotReady,            // mq is not init
} mqStatus_t;

typedef void (*MQPacketArrived)(uint8_t *packet, uint16_t packetLen, void *arg);
typedef void *(*MQ_MALLOC)(size_t size);
typedef void (*MQ_FREE)(void *);

typedef struct {
  messageQueue_t mq;
  MQPacketArrived deliver_packet;
  void *arg;
} messageQueueHandler;

#ifdef __cplusplus
extern "C" {
#endif

void MQInit(messageQueueHandler *self, MQPacketArrived pHandlerPacket, uint16_t mq_size, uint32_t buffer_size, bool use_fixed_buffer);
void MQDeInit(messageQueueHandler *self);
void MQRecvData(messageQueueHandler *self);
mqStatus_t MQEnqueue(messageQueueHandler *self, uint8_t *data, uint16_t size, void *argv);
void PacketPoll(messageQueueHandler *mq);
void MQInitMemery(MQ_MALLOC m, MQ_FREE f);

#ifdef __cplusplus
}
#endif
#endif