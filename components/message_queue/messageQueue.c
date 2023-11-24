/*
 * @Author: ch
 * @Description:
 * @Date: 2021-04-08 18:56:55
 * @LastEditTime: 2023-10-23 18:04:06
 * @LastEditors: ch
 * @version:
 * @Reference:
 */
#include "messageQueue.h"

#include "messageQueue_config.h"

#if MQ_USE
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TAG "MQ"

static mqStatus_t isEmpty(messageQueue_t *mq);

static mqStatus_t deleteElement(messageQueue_t *mq, int idx);
static mqStatus_t dequeue(messageQueue_t *mq);
mqStatus_t enqueue(messageQueue_t *mq, uint8_t *data, uint16_t size, void *argv);

static MQ_MALLOC mq_malloc = malloc;
static MQ_FREE mq_free = free;

static mqStatus_t isEmpty(messageQueue_t *mq) {
  return mq->readIdx == mq->writeIdx && mq->queuebuf[mq->readIdx].ready == 0 ? mqStatusEmpty : mqStatusNotEmpty;
}

static mqStatus_t deleteElement(messageQueue_t *mq, int idx) {
  if (mq == NULL) {
    return mqError;
  }
  if (mq->use_fixed_buffer == 0 && mq->queuebuf[idx].data != NULL) {
    mq_free(mq->queuebuf[idx].data);
    mq->queuebuf[idx].data = NULL;
  }
  if (mq->use_fixed_buffer) {
    memset(mq->queuebuf[idx].data, 0, mq->buffer_size);
  }
  mq->queuebuf[idx].ready = 0;
  mq->queuebuf[idx].len = 0;
  mq->queuebuf[idx].argv = NULL;
  return mqOK;
}

static mqStatus_t dequeue(messageQueue_t *mq) {
  if (mq == NULL) {
    return mqError;
  }
  int nextIdx = (mq->readIdx + 1) % mq->mq_size;
  if (mq->readIdx == mq->writeIdx && mq->queuebuf[mq->readIdx].ready == 0) {
#if MQ_LOG_ENABLE
    // #if MQ_LOG_LEVEL >= MQ_LOG_LEVEL_I
    mq_log(TAG, "empty queue:%d\r\n", mq->readIdx);
// #endif
#endif
    // empty queue
    return mqError;
  }
  if (deleteElement(mq, mq->readIdx) == mqError) {
    return mqError;
  }
#if MQ_LOG_ENABLE
#if MQ_LOG_LEVEL >= MQ_LOG_LEVEL_I
  mq_log(TAG, "dequeue idx:%d\r\n", mq->readIdx);
#endif
#endif
  mq->readIdx = nextIdx;

  return mqOK;
}

mqStatus_t enqueue(messageQueue_t *mq, uint8_t *data, uint16_t size, void *argv) {
  // mq is not init.
  if (mq == NULL) {
    return mqStatusNotInit;
  }
  if (size > mq->buffer_size) {
    return mqErrorOverflow;
  }

  int nextIdx = (mq->writeIdx + 1) % mq->mq_size;
  if (nextIdx == mq->readIdx && mq->queuebuf[mq->readIdx].ready == 1) {
#if MQ_LOG_ENABLE
#if MQ_LOG_LEVEL >= MQ_LOG_LEVEL_E
    mq_log(TAG, "mq buff full,%d,%d,%d\r\n", mq->writeIdx, mq->readIdx, mq->queuebuf[0].ready);
#endif
#endif
    return mqStatusBuffFull;
  }

  mq->queuebuf[mq->writeIdx].len = size;
  if (mq->use_fixed_buffer == 0 && mq->queuebuf[mq->writeIdx].data != NULL) {
    mq_free(mq->queuebuf[mq->writeIdx].data);
    mq->queuebuf[mq->writeIdx].data = NULL;
  }
  if (size != 0) {
    if (mq->use_fixed_buffer == 0 && mq->queuebuf[mq->writeIdx].data == NULL) {
      mq->queuebuf[mq->writeIdx].data = (uint8_t *) mq_malloc(size);
    }
    if (mq->queuebuf[mq->writeIdx].data == NULL) {
      mq_log(TAG, "No memory for buffer.\r\n");
      return mqErrorNotEnoughSpace;
    }
    memcpy(mq->queuebuf[mq->writeIdx].data, data, size);
  }

  mq->queuebuf[mq->writeIdx].argv = argv;
  mq->queuebuf[mq->writeIdx].ready = 1;

#if MQ_LOG_ENABLE && MQ_LOG_LEVEL >= MQ_LOG_LEVEL_I
  mq_log(TAG, "enqueue success:writeIdx:%d,ready:%d,readIdx:%d,%d\r\n", mq->writeIdx, mq->queuebuf[mq->writeIdx].ready, mq->readIdx, mq->queuebuf[0].ready);
#endif
#if MQ_LOG_ENABLE && MQ_LOG_LEVEL >= MQ_LOG_LEVEL_D
  mq_log(TAG, "enqueue write data :\t");
  for (int i = 0; i < mq->queuebuf[mq->writeIdx].len; i++) {
    printf("%02x ", *(mq->queuebuf[mq->writeIdx].data + i));
  }
#endif
  mq->writeIdx = nextIdx;
  return mqOK;
}

static mqStatus_t GetReadyChQueueBuf_NOT_alloc(messageQueue_t *mq, uint16_t *idx) {
  if (isEmpty(mq) == mqStatusEmpty) {
    return mqError;
  }

  if (mq->queuebuf[mq->readIdx].ready == 0) {
    return mqError;
  }

  if (mq->queuebuf[mq->readIdx].len > mq->buffer_size) {
    return mqError;
  }

  *idx = mq->readIdx;
  // dequeue
  // dequeue(mq);
  return mqOK;
}

void MQInit(messageQueueHandler *self, MQPacketArrived pHandlerPacket, uint16_t mq_size, uint32_t buffer_size, bool use_fixed_buffer) {
  self->mq.mq_size = mq_size;
  self->mq.buffer_size = buffer_size;
  self->mq.use_fixed_buffer = use_fixed_buffer;
  self->mq.queuebuf = (QueueBuf_t *) mq_malloc(self->mq.mq_size * sizeof(QueueBuf_t));
  for (uint32_t i = 0; i < self->mq.mq_size; i++) {
    self->mq.queuebuf[i].data = NULL;
    self->mq.queuebuf[i].argv = NULL;
    self->mq.queuebuf[i].len = 0;
    self->mq.queuebuf[i].ready = 0;
    if (self->mq.use_fixed_buffer) {
      self->mq.queuebuf[i].data = (uint8_t *) mq_malloc(self->mq.buffer_size);
    }
  }

  self->mq.readIdx = 0;
  self->mq.writeIdx = 0;
  self->arg = self;
  self->deliver_packet = pHandlerPacket;
}

void MQDeInit(messageQueueHandler *self) {
  if (self != NULL && self->mq.queuebuf != NULL) {
    for (uint32_t i = 0; i < self->mq.mq_size; i++) {
      if (self->mq.queuebuf[i].data != NULL) {
        mq_free(self->mq.queuebuf[i].data);
      }
    }
    mq_free(self->mq.queuebuf);
  }
}

void MQRecvData(messageQueueHandler *self) {
  uint16_t idx = 0;
  if (GetReadyChQueueBuf_NOT_alloc(&self->mq, &idx) != mqOK) {
    return;
  }
  self->deliver_packet(self->mq.queuebuf[idx].data, self->mq.queuebuf[idx].len, self->mq.queuebuf[idx].argv);
  dequeue(&self->mq);
}

mqStatus_t MQEnqueue(messageQueueHandler *self, uint8_t *data, uint16_t size, void *argv) {
  mqStatus_t status = mqStatusNotReady;
  status = enqueue(&self->mq, data, size, argv);
  return status;
}

void PacketPoll(messageQueueHandler *mq) {
  MQRecvData(mq);
}

void MQInitMemery(MQ_MALLOC m, MQ_FREE f) {
  mq_malloc = m;
  mq_free = f;
}
#endif