#ifndef _USBH_VIDEO_STREAM_PARSING_H
#define _USBH_VIDEO_STREAM_PARSING_H

#include "usbh_video.h"
#ifdef __cplusplus
extern "C" {
#endif

int video_stream_process_packet(uint16_t size);
void video_stream_init_buffers(uint8_t *buffer0, uint8_t *buffer1);
void video_stream_ready_update(void);
typedef void (*videoPacketArrived)(uint8_t *data, uint32_t len);

void videoPacketArrivedCallback(videoPacketArrived callback);
#ifdef __cplusplus
}
#endif

#endif
