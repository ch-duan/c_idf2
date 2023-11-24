// UVC HOST video capture for STM32 by ILIASAM
// Search for "GRXFSIZ" to change RX FIFO size
// See mor info at "usbh_video_stram_parsing.c" file

/* Includes ------------------------------------------------------------------*/
#include "usbh_video.h"

#include <stdint.h>

#include "usbh_video_desc_parsing.h"
#include "usbh_video_stream_parsing.h"

#if USBH_USE_OS
#include "cmsis_os2.h"
#endif

static USBH_StatusTypeDef USBH_VIDEO_InterfaceInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_VIDEO_InterfaceDeInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_VIDEO_SOFProcess(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_VIDEO_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_VIDEO_CSRequest(USBH_HandleTypeDef *phost, uint8_t feature, uint8_t channel);

static USBH_StatusTypeDef USBH_VIDEO_HandleCSRequest(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_VIDEO_InputStream(USBH_HandleTypeDef *phost);
void print_Probe(uvc_stream_ctrl_t probe);
USBH_ClassTypeDef VIDEO_Class = {
    "VIDEO",
    CC_VIDEO,
    USBH_VIDEO_InterfaceInit,
    USBH_VIDEO_InterfaceDeInit,
    USBH_VIDEO_ClassRequest,
    USBH_VIDEO_Process,  // BgndProcess
    USBH_VIDEO_SOFProcess,
    NULL,
};

// This struct is used for PROBE control request ( Setup Packet )
uvc_stream_ctrl_t ProbeParams;
uvc_error_callback_t uvc_error_callback = NULL;
static uint8_t usb_restart = 0;
// Buffer to store received UVC data packet
uint8_t tmp_packet_framebuffer[UVC_RX_FIFO_SIZE_LIMIT] = {0};

void uvc_error_set_callback(uvc_error_callback_t callback) {
  uvc_error_callback = callback;
}

/** @defgroup Private_Functions
 * @{
 */

/**
 * @brief  USBH_VIDEO_InterfaceInit
 *         The function init the Video class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_VIDEO_InterfaceInit(USBH_HandleTypeDef *phost) {
  USBH_StatusTypeDef status = USBH_FAIL;
  USBH_StatusTypeDef out_status;
  VIDEO_HandleTypeDef *VIDEO_Handle;
  uint8_t interface, index;

  uint16_t ep_size_in = 0;

  interface = USBH_FindInterface(phost, CC_VIDEO, USB_SUBCLASS_VIDEOCONTROL, 0x00);

  if (interface == 0xFF) /* Not Valid Interface */
  {
    USBH_ErrLog("Cannot Find the interface for %s class.", phost->pActiveClass->Name);
    status = USBH_FAIL;
    return USBH_FAIL;
  } else {
    phost->pActiveClass->pData = (VIDEO_HandleTypeDef *) USBH_malloc(sizeof(VIDEO_HandleTypeDef));
    VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
    USBH_memset(VIDEO_Handle, 0, sizeof(VIDEO_HandleTypeDef));
    /* 1st Step:  Find IN Video Interfaces */
    out_status = USBH_VIDEO_FindStreamingIN(phost);

    if (out_status == USBH_FAIL) {
      USBH_UsrLog("%s class configuration not supported.", phost->pActiveClass->Name);
      status = USBH_FAIL;
      return USBH_FAIL;
    }

    /* 2nd Step:  Select Video Streaming Interfaces with best endpoint size*/
    for (index = 0; index < VIDEO_MAX_VIDEO_STD_INTERFACE; index++) {
      if (VIDEO_Handle->stream_in[index].valid == 1) {
        uint16_t ep_size = VIDEO_Handle->stream_in[index].EpSize;
        // if (ep_size == 512)
        if ((ep_size > ep_size_in) && (ep_size <= UVC_RX_FIFO_SIZE_LIMIT)) {
          ep_size_in = ep_size;
          VIDEO_Handle->camera.interface = VIDEO_Handle->stream_in[index].interface;
          VIDEO_Handle->camera.AltSettings = VIDEO_Handle->stream_in[index].AltSettings;
          VIDEO_Handle->camera.Ep = VIDEO_Handle->stream_in[index].Ep;
          VIDEO_Handle->camera.EpSize = VIDEO_Handle->stream_in[index].EpSize;
          VIDEO_Handle->camera.Poll = VIDEO_Handle->stream_in[index].Poll;
          VIDEO_Handle->camera.supported = 1;
        }
      }
    }
    if (ep_size_in == 0) {
      return USBH_FAIL;
    }
    USBH_DbgLog("Selected EP size: %d bytes", ep_size_in);

    /* 3rd Step:  Find and Parse Video interfaces */
    USBH_VIDEO_ParseCSDescriptors(phost);

    // /* 4rd Step:  Find desrcroptors for target settings */
    // USBH_VIDEO_AnalyseFormatDescriptors(&VIDEO_Handle->class_desc);
    // if (USBH_VIDEO_Best_bFormatIndex == -1) {
    //   status = USBH_FAIL;
    //   return USBH_FAIL;
    // }

    // int frameIdx = USBH_VIDEO_AnalyseFrameDescriptors(&VIDEO_Handle->class_desc);
    // if (frameIdx == -1) {
    //   status = USBH_FAIL;
    //   return USBH_FAIL;
    // }

    if (VIDEO_Handle->camera.supported == 1) {
      VIDEO_Handle->camera.Pipe = USBH_AllocPipe(phost, VIDEO_Handle->camera.Ep);

      /* Open pipe for IN endpoint */
      USBH_OpenPipe(phost, VIDEO_Handle->camera.Pipe, VIDEO_Handle->camera.Ep, phost->device.address, phost->device.speed, USB_EP_TYPE_ISOC,
                    VIDEO_Handle->camera.EpSize);

      USBH_LL_SetToggle(phost, VIDEO_Handle->camera.Pipe, 0);
    }
    memset(&ProbeParams, 0, sizeof(ProbeParams));
    if (uvc_get_stream_ctrl_format_size(phost, &ProbeParams, UVC_FRAME_FORMAT_MJPEG, UVC_TARGET_WIDTH, UVC_TARGET_HEIGHT, 30) == USBH_FAIL) {
      USBH_UsrLog("\r\nnot find uvc format\r\n");
    }
    USBH_UsrLog("\r\n***Get Probe***\r\n");
    print_Probe(ProbeParams);
    // uvc_stream_open_ctrl(phost, &ProbeParams);
    VIDEO_Handle->req_state = VIDEO_REQ_INIT;
    VIDEO_Handle->control_state = VIDEO_CONTROL_INIT;

    status = USBH_OK;
  }
  return status;
}

/**
 * @brief  USBH_VIDEO_InterfaceDeInit
 *         The function DeInit the Pipes used for the Video class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_VIDEO_InterfaceDeInit(USBH_HandleTypeDef *phost) {
  VIDEO_HandleTypeDef *VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;

  if (VIDEO_Handle->camera.Pipe != 0x00) {
    USBH_ClosePipe(phost, VIDEO_Handle->camera.Pipe);
    USBH_FreePipe(phost, VIDEO_Handle->camera.Pipe);
    VIDEO_Handle->camera.Pipe = 0; /* Reset the pipe as Free */
  }

  if (phost->pActiveClass->pData) {
    VIDEO_HandleTypeDef *VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
    if (VIDEO_Handle != NULL) {
      for (uint8_t i = 0; i < VIDEO_Handle->class_desc.vs_desc.format_desc_num; i++) {
        for (uint8_t j = 0; j < VIDEO_Handle->class_desc.vs_desc.format_desc[i].frame_descs_num; j++) {
          if (VIDEO_Handle->class_desc.vs_desc.format_desc[i].frame_descs[j].intervals != NULL) {
            USBH_free(VIDEO_Handle->class_desc.vs_desc.format_desc[i].frame_descs[j].intervals);
          }
        }
      }
    }
    USBH_free(phost->pActiveClass->pData);
    USBH_UsrLog("free Video Class");
    phost->pActiveClass->pData = 0;
  }
  return USBH_OK;
}

/**
 * @brief  USBH_VIDEO_ClassRequest
 *         The function is responsible for handling Standard requests
 *         for Video class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_VIDEO_ClassRequest(USBH_HandleTypeDef *phost) {
  VIDEO_HandleTypeDef *VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
  USBH_StatusTypeDef status = USBH_BUSY;
  USBH_StatusTypeDef req_status = USBH_BUSY;

  /* Switch VIDEO REQ state machine */
  switch (VIDEO_Handle->req_state) {
    case VIDEO_REQ_INIT:
    case VIDEO_REQ_SET_DEFAULT_IN_INTERFACE:
      if (VIDEO_Handle->camera.supported == 1) {
        req_status = USBH_SetInterface(phost, VIDEO_Handle->camera.interface, 0);

        if (req_status == USBH_OK) {
          VIDEO_Handle->req_state = VIDEO_REQ_IDLE;
          if (!usb_restart) {
            VIDEO_Handle->steam_in_state = VIDEO_STATE_IDLE;
          } else {
            USBH_UsrLog("uvc restart");
            VIDEO_Handle->steam_in_state = VIDEO_STATE_START;
          }
        }
      } else {
        VIDEO_Handle->req_state = VIDEO_REQ_SET_DEFAULT_IN_INTERFACE;
#if (USBH_USE_OS == 1)
        phost->os_msg = (uint32_t) USBH_URB_EVENT;
#if (osCMSIS < 0x20000U)
        (void) osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
        (void) osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
      }
      break;
    case VIDEO_REQ_CS_REQUESTS:
      if (USBH_VIDEO_HandleCSRequest(phost) == USBH_OK) {
        VIDEO_Handle->req_state = VIDEO_REQ_SET_IN_INTERFACE;
      }
      break;

    case VIDEO_REQ_SET_IN_INTERFACE:
      if (VIDEO_Handle->camera.supported == 1) {
        req_status = USBH_SetInterface(phost, VIDEO_Handle->camera.interface, VIDEO_Handle->camera.AltSettings);

        if (req_status == USBH_OK) {
          VIDEO_Handle->req_state = VIDEO_REQ_IDLE;
          VIDEO_Handle->steam_in_state = VIDEO_STATE_START_IN;
        }
      } else {
        VIDEO_Handle->req_state = VIDEO_REQ_SET_IN_INTERFACE;
#if (USBH_USE_OS == 1)
        phost->os_msg = (uint32_t) USBH_URB_EVENT;
#if (osCMSIS < 0x20000U)
        (void) osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
        (void) osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
      }
      break;

    case VIDEO_REQ_IDLE:
      phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
      status = USBH_OK;
#if (USBH_USE_OS == 1)
      phost->os_msg = (uint32_t) USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
      (void) osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
      (void) osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
      break;
    case VIDEO_REQ_STOP:
      if (VIDEO_Handle->camera.supported == 1) {
        req_status = USBH_SetInterface(phost, VIDEO_Handle->camera.interface, 0);

        if (req_status == USBH_OK) {
          VIDEO_Handle->req_state = VIDEO_REQ_IDLE;
          VIDEO_Handle->steam_in_state = VIDEO_STATE_START_IN;
        }
      } else {
        VIDEO_Handle->req_state = VIDEO_REQ_SET_IN_INTERFACE;
#if (USBH_USE_OS == 1)
        phost->os_msg = (uint32_t) USBH_URB_EVENT;
#if (osCMSIS < 0x20000U)
        (void) osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
        (void) osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
      }
      break;
    case VIDEO_REQ_RESUME:
      VIDEO_Handle->req_state = VIDEO_REQ_SET_IN_INTERFACE;
#if (USBH_USE_OS == 1)
      phost->os_msg = (uint32_t) USBH_URB_EVENT;
#if (osCMSIS < 0x20000U)
      (void) osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
      (void) osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
      break;
    default:
      break;
  }
  return status;
}

/**
 * @brief  USBH_VIDEO_CSRequest
 *         The function is responsible for handling AC Specific requests for a specific feature and channel
 *         for Video class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_VIDEO_CSRequest(USBH_HandleTypeDef *phost, uint8_t feature, uint8_t channel) {
  USBH_StatusTypeDef status = USBH_BUSY;

  return status;
}

/**
 * @brief  USBH_VIDEO_HandleCSRequest
 *         The function is responsible for handling VC Specific requests for a all features
 *         and associated channels for Video class.
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_VIDEO_HandleCSRequest(USBH_HandleTypeDef *phost) {
  USBH_StatusTypeDef status = USBH_BUSY;
  USBH_StatusTypeDef cs_status = USBH_BUSY;
  VIDEO_HandleTypeDef *VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;

  cs_status = USBH_VIDEO_CSRequest(phost, VIDEO_Handle->temp_feature, 0);

  if (cs_status != USBH_BUSY) {
  }

  return status;
}

/**
  * @brief  USBH_VIDEO_Process
  *         The function is for managing state machine for Video data transfers
            MUST be called frequently!
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_VIDEO_Process(USBH_HandleTypeDef *phost) {
  USBH_StatusTypeDef status = USBH_OK;
  VIDEO_HandleTypeDef *VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;

  if (VIDEO_Handle->camera.supported == 1) {
    USBH_VIDEO_InputStream(phost);
  }

  return status;
}

/**
 * @brief  Handle Input stream process
 * @param  phost: Host handle
 * @retval USBH Status
 */

static USBH_StatusTypeDef USBH_VIDEO_InputStream(USBH_HandleTypeDef *phost) {
  static TickType_t last_time = 0;
  USBH_StatusTypeDef status = USBH_BUSY;
  VIDEO_HandleTypeDef *VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
  USBH_URBStateTypeDef result;
  switch (VIDEO_Handle->steam_in_state) {
    case VIDEO_STATE_START_IN:
      last_time = 0;
      USBH_IsocReceiveData(phost, (uint8_t *) tmp_packet_framebuffer, VIDEO_Handle->camera.EpSize, VIDEO_Handle->camera.Pipe);
      VIDEO_Handle->steam_in_state = VIDEO_STATE_DATA_IN;
      break;
    case VIDEO_STATE_DATA_IN: {
      TickType_t cur_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
      if (last_time == 0) {
        last_time = cur_time;
      }
      result = USBH_LL_GetURBState(phost, VIDEO_Handle->camera.Pipe);
      if (result == USBH_URB_DONE) {
        last_time = cur_time;
        VIDEO_Handle->uvc_err_cnt = 0;
        VIDEO_Handle->camera.timer = phost->Timer;
        volatile uint32_t rxlen = USBH_LL_GetLastXferSize(phost, VIDEO_Handle->camera.Pipe);  // Return the last transfered packet size.
        video_stream_process_packet((uint16_t) rxlen);
        memset((void *) tmp_packet_framebuffer, 0, rxlen);
        USBH_IsocReceiveData(phost, (uint8_t *) tmp_packet_framebuffer, VIDEO_Handle->camera.EpSize, VIDEO_Handle->camera.Pipe);
      } else {
#if (USBH_USE_OS == 1U)
        phost->os_msg = (uint32_t) USBH_URB_EVENT;
#if (osCMSIS < 0x20000U)
        (void) osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
        (void) osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
      }
      if (cur_time - last_time >= 50) {
        last_time = cur_time;
        printf("50ms camera  not have pic\r\n");
        if (++VIDEO_Handle->uvc_err_cnt > 3) {
          printf("uvc is error\r\n");
          if (uvc_error_callback != NULL) {
            usb_restart = 1;
            uvc_error_callback(1);
            VIDEO_Handle->steam_in_state = VIDEO_STATE_IDLE;
          }
        } else {
          if (result == USBH_URB_IDLE) {
            USBH_IsocReceiveData(phost, (uint8_t *) tmp_packet_framebuffer, VIDEO_Handle->camera.EpSize, VIDEO_Handle->camera.Pipe);
#if (USBH_USE_OS == 1U)
            phost->os_msg = (uint32_t) USBH_URB_EVENT;
#if (osCMSIS < 0x20000U)
            (void) osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
            (void) osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
          }
        }
      }
    } break;
    case VIDEO_STATE_START: {
      static int start_cnt = 0;
      usb_restart = 0;
      if (VIDEO_Handle->camera.supported == 1) {
        status = uvc_stream_open_ctrl(phost, &ProbeParams);
        if (status == USBH_OK) {
          start_cnt = 0;
          VIDEO_Handle->steam_in_state = VIDEO_STATE_START_STREAM;
#if (USBH_USE_OS == 1)
          phost->os_msg = (uint32_t) USBH_URB_EVENT;
#if (osCMSIS < 0x20000U)
          (void) osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
          (void) osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
        } else {
          if (++start_cnt > 5) {
            if (uvc_error_callback != NULL) {
              uvc_error_callback(1);
              VIDEO_Handle->steam_in_state = VIDEO_STATE_START;
            }
            start_cnt = 0;
            // VIDEO_Handle->steam_in_state = VIDEO_STATE_START_STREAM;
#if (USBH_USE_OS == 1)
            phost->os_msg = (uint32_t) USBH_URB_EVENT;
#if (osCMSIS < 0x20000U)
            (void) osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
            (void) osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
          }
        }
      }
    } break;
    case VIDEO_STATE_START_STREAM: {
      if (VIDEO_Handle->camera.supported == 1) {
        status = USBH_SetInterface(phost, VIDEO_Handle->camera.interface, VIDEO_Handle->camera.AltSettings);
        if (status == USBH_OK) {
          printf("set usb interface %d,altsettings is %d success\r\n", VIDEO_Handle->camera.interface, VIDEO_Handle->camera.AltSettings);
          VIDEO_Handle->steam_in_state = VIDEO_STATE_START_IN;
#if (USBH_USE_OS == 1)
          phost->os_msg = (uint32_t) USBH_URB_EVENT;
#if (osCMSIS < 0x20000U)
          (void) osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
          (void) osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
        }
      }
    } break;
    case VIDEO_STATE_STOP:
      if (VIDEO_Handle->camera.supported == 1) {
        status = USBH_SetInterface(phost, VIDEO_Handle->camera.interface, 0);
        if (status == USBH_OK) {
          VIDEO_Handle->steam_in_state = VIDEO_STATE_IDLE;
#if (USBH_USE_OS == 1)
          phost->os_msg = (uint32_t) USBH_URB_EVENT;
#if (osCMSIS < 0x20000U)
          (void) osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
          (void) osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
        }
      }
      break;
    case VIDEO_STATE_IDLE:
      break;
    case VIDEO_STATE_ERROR:
      VIDEO_Handle->steam_in_state = VIDEO_STATE_IDLE;
      VIDEO_Handle->req_state = VIDEO_REQ_INIT;
      phost->gState = HOST_CLASS_REQUEST;
      break;

    default:
      break;
  }

  return status;
}

/**
 * @brief  USBH_VIDEO_SOFProcess
 *         The function is for managing the SOF callback
 * @param  phost: Host handle
 * @retval USBH Status
 */
static USBH_StatusTypeDef USBH_VIDEO_SOFProcess(USBH_HandleTypeDef *phost) {
  return USBH_OK;
}

USBH_StatusTypeDef usb_control_transfer(USBH_HandleTypeDef *phost, uint8_t bmRequestType, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
                                        unsigned char *data, uint16_t wLength, unsigned int timeout) {
  phost->Control.setup.b.bmRequestType = bmRequestType;
  phost->Control.setup.b.bRequest = bRequest;
  phost->Control.setup.b.wValue.w = wValue;
  phost->Control.setup.b.wIndex.w = wIndex;
  phost->Control.setup.b.wLength.w = wLength;
  USBH_StatusTypeDef status = USBH_OK;
  uint32_t i = 0;
  do {
    status = USBH_CtlReq(phost, data, wLength);
    i += 5;
    if (i > 200) {
      return status;
    }
#if USBH_USE_OS == 1
    osDelay(5);
#endif
  } while (status == USBH_BUSY);
  return status;
}

/** @internal
 * Run a streaming control query
 * @param[in] devh UVC device
 * @param[in,out] ctrl Control block
 * @param[in] probe Whether this is a probe query or a commit query
 * @param[in] req Query type
 */
USBH_StatusTypeDef uvc_query_stream_ctrl(USBH_HandleTypeDef *phost, uvc_stream_ctrl_t *ctrl, uint8_t probe, enum uvc_req_code req) {
  uint8_t buf[34];
  size_t len;
  USBH_StatusTypeDef err = USBH_OK;
  VIDEO_HandleTypeDef *VIDEO_Handle;
  VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;

  memset(buf, 0, sizeof(buf));

  if (LE16(VIDEO_Handle->class_desc.cs_desc.HeaderDesc->bcdUVC) >= 0x0110)
    len = 34;
  else
    len = 26;

  /* prepare for a SET transfer */
  if (req == UVC_SET_CUR) {
    SHORT_TO_SW(ctrl->bmHint, buf);
    buf[2] = ctrl->bFormatIndex;
    buf[3] = ctrl->bFrameIndex;
    INT_TO_DW(ctrl->dwFrameInterval, buf + 4);
    SHORT_TO_SW(ctrl->wKeyFrameRate, buf + 8);
    SHORT_TO_SW(ctrl->wPFrameRate, buf + 10);
    SHORT_TO_SW(ctrl->wCompQuality, buf + 12);
    SHORT_TO_SW(ctrl->wCompWindowSize, buf + 14);
    SHORT_TO_SW(ctrl->wDelay, buf + 16);
    INT_TO_DW(ctrl->dwMaxVideoFrameSize, buf + 18);
    INT_TO_DW(ctrl->dwMaxPayloadTransferSize, buf + 22);

    if (len == 34) {
      INT_TO_DW(ctrl->dwClockFrequency, buf + 26);
      buf[30] = ctrl->bmFramingInfo;
      buf[31] = ctrl->bPreferedVersion;
      buf[32] = ctrl->bMinVersion;
      buf[33] = ctrl->bMaxVersion;
      /** @todo support UVC 1.1 */
    }
    USBH_UsrLog("\r\n***Set Probe***");
    print_Probe(*ctrl);
  }

  /* do the transfer */
  err = usb_control_transfer(phost, req == UVC_SET_CUR ? 0x21 : 0xA1, req, probe ? (VS_PROBE_CONTROL << 8) : (VS_COMMIT_CONTROL << 8),
                             VIDEO_Handle->camera.interface, buf, len, 0);

  /* now decode following a GET transfer */
  if (req != UVC_SET_CUR) {
    ctrl->bmHint = SW_TO_SHORT(buf);
    ctrl->bFormatIndex = buf[2];
    ctrl->bFrameIndex = buf[3];
    ctrl->dwFrameInterval = DW_TO_INT(buf + 4);
    ctrl->wKeyFrameRate = SW_TO_SHORT(buf + 8);
    ctrl->wPFrameRate = SW_TO_SHORT(buf + 10);
    ctrl->wCompQuality = SW_TO_SHORT(buf + 12);
    ctrl->wCompWindowSize = SW_TO_SHORT(buf + 14);
    ctrl->wDelay = SW_TO_SHORT(buf + 16);
    ctrl->dwMaxVideoFrameSize = DW_TO_INT(buf + 18);
    ctrl->dwMaxPayloadTransferSize = DW_TO_INT(buf + 22);
    ctrl->dwClockFrequency = DW_TO_INT(buf + 26);

    if (len == 34) {
      ctrl->bmFramingInfo = buf[30];
      ctrl->bPreferedVersion = buf[31];
      ctrl->bMinVersion = buf[32];
      ctrl->bMaxVersion = buf[33];
      /** @todo support UVC 1.1 */
    }

    // /* fix up block for cameras that fail to set dwMax* */
    // if (ctrl->dwMaxVideoFrameSize == 0) {
    //   uvc_frame_desc_t *frame = uvc_find_frame_desc(devh, ctrl->bFormatIndex, ctrl->bFrameIndex);

    //   if (frame) {
    //     ctrl->dwMaxVideoFrameSize = frame->dwMaxVideoFrameBufferSize;
    //   }
    // }
    USBH_UsrLog("\r\n***Get Probe***");
    print_Probe(*ctrl);
  }

  return err;
}

USBH_StatusTypeDef uvc_probe_stream_ctrl(USBH_HandleTypeDef *phost, uvc_stream_ctrl_t *ctrl) {
  uvc_query_stream_ctrl(phost, ctrl, 1, UVC_SET_CUR);
  uvc_query_stream_ctrl(phost, ctrl, 1, UVC_GET_CUR);
  return USBH_OK;
}

USBH_StatusTypeDef uvc_get_stream_ctrl_format_size(USBH_HandleTypeDef *phost, uvc_stream_ctrl_t *ctrl, uvc_frame_format_t format, int width, int height,
                                                   int fps) {
  VIDEO_HandleTypeDef *VIDEO_Handle;
  VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
  for (uint8_t format_idx = 0; format_idx < VIDEO_Handle->class_desc.vs_desc.format_desc_num; format_idx++) {
    uvc_format_desc_t *format_desc = &VIDEO_Handle->class_desc.vs_desc.format_desc[format_idx];
    if (format_desc == NULL || format_desc->bLength == 0)
      continue;
    if (!_uvc_frame_format_matches_guid(format, format_desc->guidFormat))
      continue;
    for (uint8_t frame_idx = 0; frame_idx < format_desc->frame_descs_num; frame_idx++) {
      uvc_frame_desc_t *frame = &format_desc->frame_descs[frame_idx];
      if (frame->wWidth != width || frame->wHeight != height)
        continue;
      uint32_t *interval;

      ctrl->bInterfaceNumber = VIDEO_Handle->camera.interface;
      if (frame->intervals) {
        for (interval = frame->intervals; *interval; ++interval) {
          // allow a fps rate of zero to mean "accept first rate available"
          if (10000000 / *interval == (unsigned int) fps || fps == 0) {
            ctrl->bmHint = (1 << 0); /* don't negotiate interval */
            ctrl->bFormatIndex = format_desc->bFormatIndex;
            ctrl->bFrameIndex = frame->bFrameIndex;
            ctrl->dwFrameInterval = *interval;

            goto found;
          }
        }
      } else {
        uint32_t interval_100ns = 10000000 / fps;
        uint32_t interval_offset = interval_100ns - frame->dwMinFrameInterval;

        if (interval_100ns >= frame->dwMinFrameInterval && interval_100ns <= frame->dwMaxFrameInterval &&
            !(interval_offset && (interval_offset % frame->dwFrameIntervalStep))) {
          ctrl->bmHint = (1 << 0);
          ctrl->bFormatIndex = format_desc->bFormatIndex;
          ctrl->bFrameIndex = frame->bFrameIndex;
          ctrl->dwFrameInterval = interval_100ns;

          goto found;
        }
      }
      uvc_query_stream_ctrl(phost, ctrl, 1, UVC_GET_MAX);
    }
  }
  return USBH_FAIL;
found:
  return uvc_probe_stream_ctrl(phost, ctrl);
}

USBH_StatusTypeDef uvc_stream_open_ctrl(USBH_HandleTypeDef *phost, uvc_stream_ctrl_t *ctrl) {
  return uvc_query_stream_ctrl(phost, ctrl, 0, UVC_SET_CUR);
}

void print_Probe(uvc_stream_ctrl_t ctrl) {
  USBH_UsrLog("bmHint: %x", ctrl.bmHint);
  USBH_UsrLog("bFormatIndex: %d", ctrl.bFormatIndex);
  USBH_UsrLog("bFrameIndex: %d", ctrl.bFrameIndex);
  USBH_UsrLog("dwFrameInterval: %u", ctrl.dwFrameInterval);
  USBH_UsrLog("wKeyFrameRate: %d", ctrl.wKeyFrameRate);
  USBH_UsrLog("wPFrameRate: %d", ctrl.wPFrameRate);
  USBH_UsrLog("wCompQuality: %d", ctrl.wCompQuality);
  USBH_UsrLog("wCompWindowSize: %d", ctrl.wCompWindowSize);
  USBH_UsrLog("wDelay: %d", ctrl.wDelay);
  USBH_UsrLog("dwMaxVideoFrameSize: %u", ctrl.dwMaxVideoFrameSize);
  USBH_UsrLog("dwMaxPayloadTransferSize: %u", ctrl.dwMaxPayloadTransferSize);
  USBH_UsrLog("dwClockFrequency: %u", ctrl.dwClockFrequency);
  USBH_UsrLog("bmFramingInfo: %u", ctrl.bmFramingInfo);
  USBH_UsrLog("bMinVersion: %u", ctrl.bMinVersion);
  USBH_UsrLog("bMaxVersion: %u", ctrl.bMaxVersion);
}

void uvc_stream_stop(USBH_HandleTypeDef *phost) {
  VIDEO_HandleTypeDef *VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
  if (VIDEO_Handle->steam_in_state == VIDEO_STATE_STOP || VIDEO_Handle->steam_in_state == VIDEO_STATE_IDLE) {
    return;
  }
  VIDEO_Handle->steam_in_state = VIDEO_STATE_STOP;
  phost->os_msg = USBH_URB_EVENT;
  osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
  USBH_UsrLog("uvc stream stop");
}

void uvc_stream_resume(USBH_HandleTypeDef *phost) {
  VIDEO_HandleTypeDef *VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
  USBH_UsrLog("uvc stream resume");
  if (VIDEO_Handle->steam_in_state == VIDEO_STATE_DATA_IN) {
    return;
  }
  VIDEO_Handle->steam_in_state = VIDEO_STATE_START;
  phost->os_msg = USBH_URB_EVENT;
  int32_t s = osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
  USBH_UsrLog("usb resume :%d\r\n", s);
}

void uvc_deinit(USBH_HandleTypeDef *phost) {
  USBH_DeInit(phost);
  VIDEO_HandleTypeDef *VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
  if (VIDEO_Handle != NULL) {
    for (uint8_t i = 0; i < VIDEO_Handle->class_desc.vs_desc.format_desc_num; i++) {
      for (uint8_t j = 0; j < VIDEO_Handle->class_desc.vs_desc.format_desc[i].frame_descs_num; j++) {
        if (VIDEO_Handle->class_desc.vs_desc.format_desc[i].frame_descs[j].intervals != NULL) {
          USBH_free(VIDEO_Handle->class_desc.vs_desc.format_desc[i].frame_descs[j].intervals);
        }
      }
    }
  }

  if (VIDEO_Handle->camera.Pipe != 0x00) {
    USBH_ClosePipe(phost, VIDEO_Handle->camera.Pipe);
    USBH_FreePipe(phost, VIDEO_Handle->camera.Pipe);
    VIDEO_Handle->camera.Pipe = 0; /* Reset the pipe as Free */
  }

  if (phost->pActiveClass->pData) {
    VIDEO_HandleTypeDef *VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
    if (VIDEO_Handle != NULL) {
      for (uint8_t i = 0; i < VIDEO_Handle->class_desc.vs_desc.format_desc_num; i++) {
        for (uint8_t j = 0; j < VIDEO_Handle->class_desc.vs_desc.format_desc[i].frame_descs_num; j++) {
          if (VIDEO_Handle->class_desc.vs_desc.format_desc[i].frame_descs[j].intervals != NULL) {
            USBH_free(VIDEO_Handle->class_desc.vs_desc.format_desc[i].frame_descs[j].intervals);
          }
        }
      }
    }
    USBH_free(phost->pActiveClass->pData);
    USBH_UsrLog("free Video Class");
    phost->pActiveClass->pData = 0;
  }
  USBH_memset(phost, 0, sizeof(USBH_HandleTypeDef));
}
