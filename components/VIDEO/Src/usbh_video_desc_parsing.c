// Parsing UVC descriptors

#include "usbh_video_desc_parsing.h"

#include "usbh_conf.h"

void printf_frame(uvc_frame_desc_t frame_desc);
void printf_format(uvc_format_desc_t format_desc);

char *descriptor_subtype[] = {
    "UVC_VS_UNDEFINED",          "UVC_VS_INPUT_HEADER",          "UVC_VS_OUTPUT_HEADER",     "UVC_VS_STILL_IMAGE_FRAME",   "UVC_VS_FORMAT_UNCOMPRESSED",
    "UVC_VS_FRAME_UNCOMPRESSED", "UVC_VS_FORMAT_MJPEG",          "UVC_VS_FRAME_MJPEG",       "UVC_VS_FORMAT_MPEG2TS",      "UVC_VS_FORMAT_DV",
    "UVC_VS_COLORFORMAT",        "UVC_VS_FORMAT_FRAME_BASED",    "UVC_VS_FRAME_FRAME_BASED", "UVC_VS_FORMAT_STREAM_BASED", "UVC_VS_FORMAT_H264",
    "UVC_VS_FRAME_H264",         "UVC_VS_FORMAT_H264_SIMULCAST", "UVC_VS_FORMAT_VP8",        "UVC_VS_FRAME_VP8",           "UVC_VS_FORMAT_VP8_SIMULCAST"};

// Target UVC mode
USBH_VIDEO_TargetFormat_t USBH_VIDEO_Target_Format = UVC_CAPTURE_MODE;

int USBH_VIDEO_Target_Width = UVC_TARGET_WIDTH;    // Width in pixels
int USBH_VIDEO_Target_Height = UVC_TARGET_HEIGHT;  // Height in pixels

// Value of "bFormatIndex" for target settings (set in USBH_VIDEO_AnalyseFormatDescriptors)
int USBH_VIDEO_Best_bFormatIndex = -1;

// Value of "bFrameIndex" for target settings (set in USBH_VIDEO_AnalyseFrameDescriptors)
int USBH_VIDEO_Best_bFrameIndex = -1;
uint32_t USBH_VIDEO_Best_dwDefaultFrameInterval = 333333;

/**
 * @brief  Find IN Video Streaming interfaces
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_VIDEO_FindStreamingIN(USBH_HandleTypeDef *phost) {
  uint8_t interface, alt_settings;
  USBH_StatusTypeDef status = USBH_FAIL;
  VIDEO_HandleTypeDef *VIDEO_Handle;

  VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;

  // Look For VIDEOSTREAMING IN interface (data FROM camera)
  alt_settings = 0;
  for (interface = 0; interface < USBH_MAX_NUM_INTERFACES; interface++) {
    if ((phost->device.CfgDesc.Itf_Desc[interface].bInterfaceClass == CC_VIDEO) &&
        (phost->device.CfgDesc.Itf_Desc[interface].bInterfaceSubClass == USB_SUBCLASS_VIDEOSTREAMING)) {
      if ((phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80) &&  // is IN EP
          (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize > 0)) {
        VIDEO_Handle->stream_in[alt_settings].Ep = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
        VIDEO_Handle->stream_in[alt_settings].EpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
        VIDEO_Handle->stream_in[alt_settings].interface = phost->device.CfgDesc.Itf_Desc[interface].bInterfaceNumber;
        VIDEO_Handle->stream_in[alt_settings].AltSettings = phost->device.CfgDesc.Itf_Desc[interface].bAlternateSetting;
        VIDEO_Handle->stream_in[alt_settings].Poll = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bInterval;
        VIDEO_Handle->stream_in[alt_settings].valid = 1;
        alt_settings++;
      }
    }
  }

  if (alt_settings > 0) {
    status = USBH_OK;
  }

  return status;
}

/**
 * @brief  Parse VC and interfaces Descriptors
 * @param  phost: Host handle
 * @retval USBH Status
 */
USBH_StatusTypeDef USBH_VIDEO_ParseCSDescriptors(USBH_HandleTypeDef *phost) {
  // Pointer to header of descriptor
  USBH_DescHeader_t *pdesc;
  uint16_t ptr;
  int8_t itf_index = 0;
  int8_t itf_number = 0;
  int8_t alt_setting;
  VIDEO_HandleTypeDef *VIDEO_Handle;

  VIDEO_Handle = (VIDEO_HandleTypeDef *) phost->pActiveClass->pData;
  pdesc = (USBH_DescHeader_t *) (phost->device.CfgDesc_Raw);
  ptr = USB_LEN_CFG_DESC;

  VIDEO_Handle->class_desc.InputTerminalNum = 0;
  VIDEO_Handle->class_desc.OutputTerminalNum = 0;
  VIDEO_Handle->class_desc.ASNum = 0;

  while (ptr < phost->device.CfgDesc.wTotalLength) {
    pdesc = USBH_GetNextDesc((uint8_t *) pdesc, &ptr);

    switch (pdesc->bDescriptorType) {
      case USB_DESC_TYPE_INTERFACE:
        itf_number = *((uint8_t *) pdesc + 2);  // bInterfaceNumber
        alt_setting = *((uint8_t *) pdesc + 3);
        itf_index = USBH_FindInterfaceIndex(phost, itf_number, alt_setting);
        break;

      case USB_DESC_TYPE_CS_INTERFACE:  // 0x24 - Class specific descriptor
        if (itf_number <= phost->device.CfgDesc.bNumInterfaces) {
          ParseCSDescriptors(&VIDEO_Handle->class_desc, phost->device.CfgDesc.Itf_Desc[itf_index].bInterfaceSubClass, (uint8_t *) pdesc);
        }
        break;

      default:
        break;
    }
  }
  return USBH_OK;
}

USBH_StatusTypeDef uvc_parse_vs_frame_uncompressed(uvc_frame_desc_t *frame, const unsigned char *block, size_t block_size) {
  const unsigned char *p;
  int i;
  frame->bLength = block[0];
  frame->bDescriptorType = block[1];
  frame->bDescriptorSubtype = block[2];
  frame->bFrameIndex = block[3];
  frame->bmCapabilities = block[4];
  frame->wWidth = block[5] + (block[6] << 8);
  frame->wHeight = block[7] + (block[8] << 8);
  frame->dwMinBitRate = DW_TO_INT(&block[9]);
  frame->dwMaxBitRate = DW_TO_INT(&block[13]);
  frame->dwMaxVideoFrameBufferSize = DW_TO_INT(&block[17]);
  frame->dwDefaultFrameInterval = DW_TO_INT(&block[21]);
  frame->bFrameIntervalType = block[25];

  if (block[25] == 0) {
    frame->dwMinFrameInterval = DW_TO_INT(&block[26]);
    frame->dwMaxFrameInterval = DW_TO_INT(&block[30]);
    frame->dwFrameIntervalStep = DW_TO_INT(&block[34]);
  } else {
    frame->intervals = (uint32_t *) USBH_malloc(block[25] * sizeof(frame->intervals[0]));
    USBH_memset(frame->intervals, 0, block[25]);
    p = &block[26];

    for (i = 0; i < block[25]; ++i) {
      frame->intervals[i] = DW_TO_INT(p);
      p += 4;
    }
  }

  return USBH_OK;
}

/** @internal
 * @brief Parse a VideoStreaming MJPEG format block.
 * @ingroup device
 */
USBH_StatusTypeDef uvc_parse_vs_format_mjpeg(uvc_format_desc_t *format, const unsigned char *block, size_t block_size) {
  format->bLength = block[0];
  format->bDescriptorType = block[1];
  format->bDescriptorSubtype = block[2];
  format->bFormatIndex = block[3];
  format->bNumFrameDescriptors = block[4];
  memcpy(format->fourccFormat, "MJPG", 4);
  format->bmFlags = block[5];
  format->bBitsPerPixel = 0;
  format->bDefaultFrameIndex = block[6];
  format->bAspectRatioX = block[7];
  format->bAspectRatioY = block[8];
  format->bmInterlaceFlags = block[9];
  format->bCopyProtect = block[10];

  return USBH_OK;
}

/** @internal
 * @brief Parse a VideoStreaming uncompressed format block.
 * @ingroup device
 */
USBH_StatusTypeDef uvc_parse_vs_format_uncompressed(uvc_format_desc_t *format, const unsigned char *block, size_t block_size) {
  format->bLength = block[0];
  format->bDescriptorType = block[1];
  format->bDescriptorSubtype = block[2];
  format->bFormatIndex = block[3];
  format->bNumFrameDescriptors = block[4];
  memcpy(format->guidFormat, &block[5], 16);
  format->bBitsPerPixel = block[21];
  format->bDefaultFrameIndex = block[22];
  format->bAspectRatioX = block[23];
  format->bAspectRatioY = block[24];
  format->bmInterlaceFlags = block[25];
  format->bCopyProtect = block[26];
  return USBH_OK;
}

/**
 * @brief  Parse Class specific descriptor
 * @param  vs_subclass: bInterfaceSubClass
 * pdesc: current desctiptor
 * @retval USBH Status
 */
USBH_StatusTypeDef ParseCSDescriptors(VIDEO_ClassSpecificDescTypedef *class_desc, uint8_t vs_subclass, uint8_t *pdesc) {
  uint8_t desc_number = 0;

  if (vs_subclass == USB_SUBCLASS_VIDEOCONTROL) {
    switch (pdesc[2]) {
      case UVC_VC_HEADER:
        class_desc->cs_desc.HeaderDesc = (VIDEO_HeaderDescTypeDef *) pdesc;
        break;

      case UVC_VC_INPUT_TERMINAL:
        class_desc->cs_desc.InputTerminalDesc[class_desc->InputTerminalNum++] = (VIDEO_ITDescTypeDef *) pdesc;
        break;

      case UVC_VC_OUTPUT_TERMINAL:
        class_desc->cs_desc.OutputTerminalDesc[class_desc->OutputTerminalNum++] = (VIDEO_OTDescTypeDef *) pdesc;
        break;

      case UVC_VC_SELECTOR_UNIT:
        class_desc->cs_desc.SelectorUnitDesc[class_desc->SelectorUnitNum++] = (VIDEO_SelectorDescTypeDef *) pdesc;
        break;

      default:
        break;
    }
  } else if (vs_subclass == USB_SUBCLASS_VIDEOSTREAMING) {
    switch (pdesc[2]) {
      case UVC_VS_INPUT_HEADER:
        if (class_desc->InputHeaderNum < VIDEO_MAX_NUM_IN_HEADER)
          class_desc->vs_desc.InputHeader[class_desc->InputHeaderNum++] = (VIDEO_InHeaderDescTypeDef *) pdesc;
        break;

      case UVC_VS_FORMAT_UNCOMPRESSED:
        if (class_desc->vs_desc.format_desc_num < VIDEO_MAX_FORMAT_DESC) {
          uvc_parse_vs_format_uncompressed(&class_desc->vs_desc.format_desc[class_desc->vs_desc.format_desc_num], pdesc, pdesc[0]);
          printf_format(class_desc->vs_desc.format_desc[class_desc->vs_desc.format_desc_num]);
          class_desc->vs_desc.format_desc_num++;
        }
        break;

      case UVC_VS_FORMAT_MJPEG:
        if (class_desc->vs_desc.format_desc_num < VIDEO_MAX_FORMAT_DESC) {
          uvc_parse_vs_format_mjpeg(&class_desc->vs_desc.format_desc[class_desc->vs_desc.format_desc_num], pdesc, pdesc[0]);
          printf_format(class_desc->vs_desc.format_desc[class_desc->vs_desc.format_desc_num]);
          class_desc->vs_desc.format_desc_num++;
        }
        break;
      case UVC_VS_FRAME_UNCOMPRESSED:
      case UVC_VS_FRAME_MJPEG: {
        int format_number = class_desc->vs_desc.format_desc_num == 0 ? 0 : class_desc->vs_desc.format_desc_num - 1;
        desc_number = class_desc->vs_desc.format_desc[format_number].frame_descs_num;
        if (desc_number < VIDEO_MAX_FORMAT_FRAME) {
          /* In order to prevent data errors caused by the data size end on some platforms, manually calculate the data exceeding 1 byte */
          uvc_parse_vs_frame_uncompressed(&class_desc->vs_desc.format_desc[format_number].frame_descs[desc_number], pdesc, pdesc[0]);
          USBH_DbgLog("MJPEG Frame detected: %d x %d", class_desc->vs_desc.format_desc[format_number].frame_descs[desc_number].wWidth,
                      class_desc->vs_desc.format_desc[format_number].frame_descs[desc_number].wHeight);
          printf_frame(class_desc->vs_desc.format_desc[format_number].frame_descs[desc_number]);
          class_desc->vs_desc.format_desc[format_number].frame_descs_num++;
        }
      } break;

      default:
        break;
    }
  }

  return USBH_OK;
}

/*
 * Check if camera have needed Format descriptor (base for MJPEG/Uncompressed frames)
 */
void USBH_VIDEO_AnalyseFormatDescriptors(VIDEO_ClassSpecificDescTypedef *class_desc) {
  // USBH_VIDEO_Best_bFormatIndex = -1;

  // if (USBH_VIDEO_Target_Format == USBH_VIDEO_MJPEG) {
  //   for (int i = 0; i < class_desc->vs_desc.format_desc_num; i++) {
  //     printf_format(class_desc->vs_desc.MJPEGFormat[i]);
  //   }
  //   if (class_desc->vs_desc.format_desc_num != 1) {
  //     USBH_ErrLog("Not supported MJPEG descriptors number: %d", class_desc->vs_desc.format_desc_num);
  //   } else {
  //     VIDEO_MJPEGFormatDescTypeDef *mjpeg_format_desc;
  //     mjpeg_format_desc = class_desc->vs_desc.MJPEGFormat[0];
  //     USBH_VIDEO_Best_bFormatIndex = mjpeg_format_desc->bFormatIndex;
  //   }
  //   return;
  // } else if (USBH_VIDEO_Target_Format == USBH_VIDEO_YUY2) {
  //   if (class_desc->UncompFormatNum != 1) {
  //     USBH_ErrLog("Not supported UNCOMP descriptors number: %d", class_desc->UncompFormatNum);
  //     return;
  //   } else {
  //     // Camera have a single Format descriptor, so we need to check if this descriptor is really YUY2
  //     VIDEO_UncompFormatDescTypeDef *uncomp_format_desc;
  //     uncomp_format_desc = class_desc->vs_desc.UncompFormat[0];

  //     if (memcmp(&uncomp_format_desc->guidFormat, "YUY2", 4) != 0) {
  //       USBH_ErrLog("Not supported UNCOMP descriptor type");
  //       return;
  //     } else {
  //       // Found!
  //       USBH_VIDEO_Best_bFormatIndex = uncomp_format_desc->bFormatIndex;
  //     }
  //   }
  // }
}

/*
 * Check if camera have needed Frame descriptor (whith target image width)
 */
int USBH_VIDEO_AnalyseFrameDescriptors(VIDEO_ClassSpecificDescTypedef *class_desc) {
  // USBH_VIDEO_Best_bFrameIndex = -1;
  int result = -1;

  // if (USBH_VIDEO_Target_Format == USBH_VIDEO_MJPEG) {
  //   for (uint8_t i = 0; i < class_desc->vs_desc.format_desc[format_number].frame_descs_num; i++) {
  //     VIDEO_MJPEGFrameDescTypeDef *mjpeg_frame_desc;
  //     mjpeg_frame_desc = class_desc->vs_desc.MJPEGFrame[i];
  //     printf_frame(class_desc->vs_desc.MJPEGFrame[i]);
  //     if ((mjpeg_frame_desc->wWidth == USBH_VIDEO_Target_Width) && (mjpeg_frame_desc->wHeight == USBH_VIDEO_Target_Height)) {
  //       // Found!
  //       USBH_VIDEO_Best_bFrameIndex = mjpeg_frame_desc->bFrameIndex;
  //       USBH_VIDEO_Best_dwDefaultFrameInterval = mjpeg_frame_desc->dwDefaultFrameInterval;
  //       USBH_UsrLog("*** found frame ***\r\n");
  //       printf_frame(class_desc->vs_desc.MJPEGFrame[i]);
  //       return i;
  //     }
  //   }
  // } else if (USBH_VIDEO_Target_Format == USBH_VIDEO_YUY2) {
  //   for (uint8_t i = 0; i < class_desc->UncompFrameNum; i++) {
  //     VIDEO_UncompFrameDescTypeDef *uncomp_frame_desc;
  //     uncomp_frame_desc = class_desc->vs_desc.UncompFrame[i];
  //     printf_frame(class_desc->vs_desc.MJPEGFrame[i]);
  //     if ((uncomp_frame_desc->wWidth == USBH_VIDEO_Target_Width) && (uncomp_frame_desc->wHeight == USBH_VIDEO_Target_Height)) {
  //       // Found!
  //       USBH_VIDEO_Best_bFrameIndex = uncomp_frame_desc->bFrameIndex;
  //       USBH_VIDEO_Best_dwDefaultFrameInterval = uncomp_frame_desc->dwDefaultFrameInterval;
  //       USBH_UsrLog("*** found frame ***\r\n");
  //       printf_frame(class_desc->vs_desc.MJPEGFrame[i]);
  //       return i;
  //     }
  //   }
  // }
  return result;
}

void printf_frame(uvc_frame_desc_t frame_desc) {
  USBH_UsrLog("VideoStreaming Interface Descriptor");
  USBH_UsrLog(" bLength:%d", frame_desc.bLength);
  USBH_UsrLog(" bDescriptorType:%d", frame_desc.bDescriptorType);
  USBH_UsrLog(" bDescriptorSubtype:%d (%s)", frame_desc.bDescriptorSubtype,
              frame_desc.bDescriptorSubtype <= UVC_VS_FORMAT_VP8_SIMULCAST ? descriptor_subtype[frame_desc.bDescriptorSubtype] : "not found");
  USBH_UsrLog(" bFrameIndex:%d", frame_desc.bFrameIndex);
  USBH_UsrLog(" bmCapabilities:%d", frame_desc.bmCapabilities);
  USBH_UsrLog(" wWidth:%d", frame_desc.wWidth);
  USBH_UsrLog(" wHeight:%d", frame_desc.wHeight);
  USBH_UsrLog(" dwMinBitRate:%u", frame_desc.dwMinBitRate);
  USBH_UsrLog(" dwMaxBitRate:%u", frame_desc.dwMaxBitRate);
  USBH_UsrLog(" dwMaxVideoFrameBufferSize:%u", frame_desc.dwMaxVideoFrameBufferSize);
  USBH_UsrLog(" dwDefaultFrameInterval:%u", frame_desc.dwDefaultFrameInterval);
  USBH_UsrLog(" bFrameIntervalType:%d\r\n", frame_desc.bFrameIntervalType);
}
void printf_format(uvc_format_desc_t format_desc) {
  USBH_UsrLog("VideoStreaming Interface Descriptor");
  USBH_UsrLog(" bLength:%d", format_desc.bLength);
  USBH_UsrLog(" bDescriptorType:%d", format_desc.bDescriptorType);
  USBH_UsrLog(" bDescriptorSubtype:%d (%s)", format_desc.bDescriptorSubtype,
              format_desc.bDescriptorSubtype <= UVC_VS_FORMAT_VP8_SIMULCAST ? descriptor_subtype[format_desc.bDescriptorSubtype] : "not found");
  USBH_UsrLog(" bFormatIndex:%d", format_desc.bFormatIndex);
  USBH_UsrLog(" bNumFrameDescriptors:%d", format_desc.bNumFrameDescriptors);
  USBH_UsrLog(" bmFlags:%d", format_desc.bmFlags);
  USBH_UsrLog(" bDefaultFrameIndex:%d", format_desc.bDefaultFrameIndex);
  USBH_UsrLog(" bAspectRatioX:%d", format_desc.bAspectRatioX);
  USBH_UsrLog(" bmInterlaceFlags:%d", format_desc.bmInterlaceFlags);
  USBH_UsrLog(" bCopyProtect:%d", format_desc.bCopyProtect);
}
