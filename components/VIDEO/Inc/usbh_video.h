
/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_VIDEO_H
#define __USBH_VIDEO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"
#include "m_malloc.h"

/** Converts an unaligned four-byte little-endian integer into an int32 */
#define DW_TO_INT(p) ((p)[0] | ((p)[1] << 8) | ((p)[2] << 16) | ((p)[3] << 24))
/** Converts an unaligned two-byte little-endian integer into an int16 */
#define SW_TO_SHORT(p) ((p)[0] | ((p)[1] << 8))
/** Converts an int16 into an unaligned two-byte little-endian integer */
#define SHORT_TO_SW(s, p) \
  (p)[0] = (s);           \
  (p)[1] = (s) >> 8;
/** Converts an int32 into an unaligned four-byte little-endian integer */
#define INT_TO_DW(i, p) \
  (p)[0] = (i);         \
  (p)[1] = (i) >> 8;    \
  (p)[2] = (i) >> 16;   \
  (p)[3] = (i) >> 24;

/** Selects the nth item in a doubly linked list. n=-1 selects the last item. */
#define DL_NTH(head, out, n)               \
  do {                                     \
    int dl_nth_i = 0;                      \
    LDECLTYPE(head) dl_nth_p = (head);     \
    if ((n) < 0) {                         \
      while (dl_nth_p && dl_nth_i > (n)) { \
        dl_nth_p = dl_nth_p->prev;         \
        dl_nth_i--;                        \
      }                                    \
    } else {                               \
      while (dl_nth_p && dl_nth_i < (n)) { \
        dl_nth_p = dl_nth_p->next;         \
        dl_nth_i++;                        \
      }                                    \
    }                                      \
    (out) = dl_nth_p;                      \
  } while (0);

// Maximum endpoint size in bytes
#define UVC_RX_FIFO_SIZE_LIMIT 3072

// Image width
#define UVC_TARGET_WIDTH  320
#define UVC_TARGET_HEIGHT 240

#define UVC_CAPTURE_MODE  USBH_VIDEO_MJPEG
// #define UVC_CAPTURE_MODE                USBH_VIDEO_YUY2

// Uncompressed image frame size in byte
#define UVC_UNCOMP_FRAME_SIZE (UVC_TARGET_WIDTH * UVC_TARGET_HEIGHT * 2)

#if USE_EXTERNAL_SRAM == 1
#define UVC_MAX_FRAME_SIZE (UVC_UNCOMP_FRAME_SIZE)
#else
#define UVC_MAX_FRAME_SIZE 20 * 1024
#endif
// TODO - UVC_MAX_FRAME_SIZE for MJPEG mode can be smaller.
// Needed value is send by camera - see "USBH_VS_GetCur" - dwMaxVideoFrameSize

typedef enum {
  USBH_VIDEO_MJPEG = 0,
  USBH_VIDEO_YUY2,
} USBH_VIDEO_TargetFormat_t;

/* States for VIDEO State Machine */
typedef enum {
  VIDEO_INIT = 0,
  VIDEO_IDLE,
  VIDEO_CS_REQUESTS,
  VIDEO_SET_DEFAULT_FEATURE_UNIT,
  VIDEO_SET_INTERFACE,
  VIDEO_SET_STREAMING_INTERFACE,
  VIDEO_SET_CUR1,
  VIDEO_GET_RES,
  VIDEO_GET_CUR1,
  VIDEO_SET_CUR2,
  VIDEO_GET_CUR2,
  VIDEO_SET_CUR3,
  VIDEO_SET_INTERFACE0,
  VIDEO_SET_INTERFACE1,
  VIDEO_SET_INTERFACE2,
  VIDEO_ISOC_OUT,
  VIDEO_ISOC_IN,
  VIDEO_ISOC_POLL,
  VIDEO_ERROR,
} VIDEO_StateTypeDef;

typedef enum {
  VIDEO_REQ_INIT = 1,
  VIDEO_REQ_IDLE,
  VIDEO_REQ_SET_DEFAULT_IN_INTERFACE,
  VIDEO_REQ_SET_IN_INTERFACE,
  VIDEO_REQ_CS_REQUESTS,
  VIDEO_REQ_STOP,
  VIDEO_REQ_RESUME,
  VIDEO_REQ_ERROR,
} VIDEO_ReqStateTypeDef;

typedef enum {
  VIDEO_CONTROL_INIT = 1,
  VIDEO_CONTROL_CHANGE,
  VIDEO_CONTROL_IDLE,
} VIDEO_ControlStateTypeDef;

/**
 * @}
 */

/** @defgroup USBH_VIDEO_CORE_Exported_Defines
 * @{
 */

/*Video Interface Subclass Codes*/
#define CC_VIDEO 0x0E

/* Video Interface Subclass Codes */
#define USB_SUBCLASS_VIDEOCONTROL               0x01
#define USB_SUBCLASS_VIDEOSTREAMING             0x02
#define USB_SUBCLASS_VIDEO_INTERFACE_COLLECTION 0x03

// Class specific
#ifndef USB_DESC_TYPE_CS_INTERFACE
#define USB_DESC_TYPE_CS_INTERFACE 0x24
#endif
#ifndef USB_DESC_TYPE_CS_ENDPOINT
#define USB_DESC_TYPE_CS_ENDPOINT 0x25
#endif
// Video Class-Specific VideoControl Interface Descriptor Subtypes
// (USB_Video_Class_1.1.pdf, A.5 Video Class-Specific VC Interface Descriptor Subtypes)
#define UVC_VC_HEADER          0x01
#define UVC_VC_INPUT_TERMINAL  0x02
#define UVC_VC_OUTPUT_TERMINAL 0x03
#define UVC_VC_SELECTOR_UNIT   0x04
#define UVC_VC_PROCESSING_UNIT 0x05
#define UVC_VC_EXTENSION_UNIT  0x06

// Video Class-Specific VideoStreaming Interface Descriptor Subtypes
// (USB_Video_Class_1.1.pdf, A.6 Video Class-Specific VS Interface Descriptor Subtypes)
// #define UVC_VS_UNDEFINED             0x00
// #define UVC_VS_INPUT_HEADER          0x01
// #define UVC_VS_OUTPUT_HEADER         0x02
// #define UVC_VS_STILL_IMAGE_FRAME     0x03
// #define UVC_VS_FORMAT_UNCOMPRESSED   0x04
// #define UVC_VS_FRAME_UNCOMPRESSED    0x05
// #define UVC_VS_FORMAT_MJPEG          0x06
// #define UVC_VS_FRAME_MJPEG           0x07
// #define UVC_VS_FORMAT_MPEG2TS        0x0A
// #define UVC_VS_FORMAT_DV             0x0C
// #define UVC_VS_COLORFORMAT           0x0D
// #define UVC_VS_FORMAT_FRAME_BASED    0x10
// #define UVC_VS_FRAME_FRAME_BASED     0x11
// #define UVC_VS_FORMAT_STREAM_BASED   0x12
// #define UVC_VS_FORMAT_H264           0x13
// #define UVC_VS_FRAME_H264            0x14
// #define UVC_VS_FORMAT_H264_SIMULCAST 0x15
// #define UVC_VS_FORMAT_VP8            0x16
// #define UVC_VS_FRAME_VP8             0x17
// #define UVC_VS_FORMAT_VP8_SIMULCAST  0x18

enum uvc_vs_desc_subtype {
  UVC_VS_UNDEFINED = 0x00,
  UVC_VS_INPUT_HEADER = 0x01,
  UVC_VS_OUTPUT_HEADER = 0x02,
  UVC_VS_STILL_IMAGE_FRAME = 0x03,
  UVC_VS_FORMAT_UNCOMPRESSED = 0x04,
  UVC_VS_FRAME_UNCOMPRESSED = 0x05,
  UVC_VS_FORMAT_MJPEG = 0x06,
  UVC_VS_FRAME_MJPEG = 0x07,
  UVC_VS_FORMAT_MPEG2TS = 0x0a,
  UVC_VS_FORMAT_DV = 0x0c,
  UVC_VS_COLORFORMAT = 0x0d,
  UVC_VS_FORMAT_FRAME_BASED = 0x10,
  UVC_VS_FRAME_FRAME_BASED = 0x11,
  UVC_VS_FORMAT_STREAM_BASED = 0x12,
  UVC_VS_FORMAT_H264 = 0x13,
  UVC_VS_FRAME_H264 = 0x14,
  UVC_VS_FORMAT_H264_SIMULCAST = 0x15,
  UVC_VS_FORMAT_VP8 = 0x16,
  UVC_VS_FRAME_VP8 = 0x17,
  UVC_VS_FORMAT_VP8_SIMULCAST = 0x18,
};

#define UVC_AS_GENERAL      0x01
#define UVC_FORMAT_TYPE     0x02
#define UVC_FORMAT_SPECIFIC 0x03

/* Video Class-Specific Endpoint Descriptor Subtypes */
#define UVC_EP_GENERAL 0x01

/* Video Class-Specific Request Codes */
#define UVC_SET_ 0x00
#define UVC_GET_ 0x80

#define UVC__CUR 0x1
#define UVC__MIN 0x2
#define UVC__MAX 0x3
#define UVC__RES 0x4
#define UVC__MEM 0x5

enum uvc_req_code {
  UVC_RC_UNDEFINED = 0x00,
  UVC_SET_CUR = 0x01,
  UVC_GET_CUR = 0x81,
  UVC_SET_MIN = (UVC_SET_ | UVC__MIN),
  UVC_GET_MIN = 0x82,
  UVC_SET_MAX = (UVC_SET_ | UVC__MAX),
  UVC_GET_MAX = 0x83,
  UVC_SET_RES = (UVC_SET_ | UVC__RES),
  UVC_GET_RES = 0x84,
  UVC_SET_MEM = (UVC_SET_ | UVC__MEM),
  UVC_GET_LEN = 0x85,
  UVC_GET_INFO = 0x86,
  UVC_GET_DEF = 0x87
};

#define UVC_GET_STAT 0xff

/* Terminals - 2.1 USB Terminal Types */
#define UVC_TERMINAL_UNDEFINED        0x100
#define UVC_TERMINAL_STREAMING        0x101
#define UVC_TERMINAL_VENDOR_SPEC      0x1FF

#define VIDEO_MAX_VIDEO_STD_INTERFACE 0x05

// Video Control Descriptor
#define VIDEO_MAX_NUM_IN_TERMINAL   10
#define VIDEO_MAX_NUM_OUT_TERMINAL  4
#define VIDEO_MAX_NUM_FEATURE_UNIT  2
#define VIDEO_MAX_NUM_SELECTOR_UNIT 2

// Video Steream Descriptor
#define VIDEO_MAX_NUM_IN_HEADER 3

// Video Steream Descriptor

#define VIDEO_MAX_FORMAT_DESC    5

#define VIDEO_MAX_FORMAT_FRAME   20

#define VIDEO_MAX_MJPEG_FORMAT   3
#define VIDEO_MAX_MJPEG_FRAME_D  10

#define VIDEO_MAX_UNCOMP_FORMAT  3
#define VIDEO_MAX_UNCOMP_FRAME_D 10

#define VIDEO_MAX_SAMFREQ_NBR    5
#define VIDEO_MAX_INTERFACE_NBR  5
#define VIDEO_MAX_CONTROLS_NBR   5

#define VS_PROBE_CONTROL         0x01
#define VS_COMMIT_CONTROL        0x02

typedef enum uvc_frame_format {
  UVC_FRAME_FORMAT_UNKNOWN = 0,
  /** Any supported format */
  UVC_FRAME_FORMAT_ANY = 0,
  UVC_FRAME_FORMAT_UNCOMPRESSED,
  UVC_FRAME_FORMAT_COMPRESSED,
  /** YUYV/YUV2/YUV422: YUV encoding with one luminance value per pixel and
   * one UV (chrominance) pair for every two pixels.
   */
  UVC_FRAME_FORMAT_YUYV,
  UVC_FRAME_FORMAT_UYVY,
  /** 24-bit RGB */
  UVC_FRAME_FORMAT_RGB,
  UVC_FRAME_FORMAT_BGR,
  /** Motion-JPEG (or JPEG) encoded images */
  UVC_FRAME_FORMAT_MJPEG,
  UVC_FRAME_FORMAT_H264,
  /** Greyscale images */
  UVC_FRAME_FORMAT_GRAY8,
  UVC_FRAME_FORMAT_GRAY16,
  /* Raw colour mosaic images */
  UVC_FRAME_FORMAT_BY8,
  UVC_FRAME_FORMAT_BA81,
  UVC_FRAME_FORMAT_SGRBG8,
  UVC_FRAME_FORMAT_SGBRG8,
  UVC_FRAME_FORMAT_SRGGB8,
  UVC_FRAME_FORMAT_SBGGR8,
  /** YUV420: NV12 */
  UVC_FRAME_FORMAT_NV12,
  /** YUV: P010 */
  UVC_FRAME_FORMAT_P010,
  /** Number of formats understood */
  UVC_FRAME_FORMAT_COUNT,
} uvc_frame_format_t;

typedef enum {
  VIDEO_STATE_IDLE = 1,
  VIDEO_STATE_START,
  VIDEO_STATE_START_STREAM,
  VIDEO_STATE_STOP,
  VIDEO_STATE_START_IN,
  VIDEO_STATE_DATA_IN,
  VIDEO_STATE_ERROR,
} VIDEO_StreamStateTypeDef;

typedef struct {
  uint8_t Ep;           // bEndpointAddress
  uint16_t EpSize;      // wMaxPacketSize
  uint8_t AltSettings;  // bAlternateSetting
  uint8_t interface;    // bInterfaceNumber
  uint8_t valid;
  uint16_t Poll;        // bInterval
} VIDEO_STREAMING_IN_HandleTypeDef;

typedef struct {
  uint8_t Ep;
  uint16_t EpSize;
  uint8_t interface;
  uint8_t AltSettings;
  uint8_t supported;

  uint8_t Pipe;
  uint8_t Poll;
  uint32_t timer;

  uint8_t asociated_as;

  uint8_t *buf;
  uint8_t *cbuf;
  uint32_t partial_ptr;

  uint32_t global_ptr;
  uint16_t frame_length;
  uint32_t total_length;
} VIDEO_InterfaceStreamPropTypeDef;

/*  Class-Specific VC Header Descriptor */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;  // must be UVC_VC_HEADER
  uint8_t bcdUVC[2];
  uint8_t wTotalLength[2];     // header+units+terminals
  uint8_t dwClockFrequency[4];
  uint8_t bInCollection;
  uint8_t baInterfaceNr[VIDEO_MAX_INTERFACE_NBR];
} VIDEO_HeaderDescTypeDef;

/* VC Input Terminal Descriptor */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;  // must be UVC_VC_INPUT_TERMINAL
  uint8_t bTerminalID;
  uint8_t wTerminalType[2];    // must be 0x0201 = (ITT_CAMERA)
  uint8_t bAssocTerminal;
  uint8_t iTerminal;
  uint8_t wObjectiveFocalLengthMin[2];
  uint8_t wObjectiveFocalLengthMax[2];
  uint8_t wOcularFocalLength[2];
  uint8_t bControlSize;
  uint8_t bmControls[3];  // in fact, size of this array if defined by "bControlSize" value
} VIDEO_ITDescTypeDef;

/* VC Output Terminal Descriptor */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;  // must be UVC_VC_INPUT_TERMINAL
  uint8_t bTerminalID;
  uint8_t wTerminalType[2];
  uint8_t bAssocTerminal;
  uint8_t bSourceID;
  uint8_t iTerminal;
} VIDEO_OTDescTypeDef;

/* Feature Descriptor */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bUnitID;
  uint8_t bSourceID;
  uint8_t bControlSize;
  uint8_t bmaControls[VIDEO_MAX_CONTROLS_NBR][2];
} VIDEO_FeatureDescTypeDef;

/*  */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bUnitID;
  uint8_t bNrInPins;
  uint8_t bSourceID0;
  uint8_t iSelector;
} VIDEO_SelectorDescTypeDef;

//**********************************************************************
// Video Stream Descriptors

/* VS Input Header Descriptor */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;  // must be UVC_VS_INPUT_HEADER
  uint8_t bNumFormats;
  uint8_t wTotalLength[2];
  uint8_t bEndPointAddress;
  uint8_t bmInfo;
  uint8_t bTerminalLink;
  uint8_t bStillCaptureMethod;
  uint8_t bTriggerSupport;
  uint8_t bTriggerUsage;
  uint8_t bControlSize;
  uint8_t bmaControls;
} VIDEO_InHeaderDescTypeDef;

/* VS MJPEG Format Descriptor */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bFormatIndex;
  uint8_t bNumFrameDescriptors;
  uint8_t bmFlags;
  uint8_t bDefaultFrameIndex;
  uint8_t bAspectRatioX;
  uint8_t bAspectRatioY;
  uint8_t bmInterlaceFlags;
  uint8_t bCopyProtect;
} VIDEO_MJPEGFormatDescTypeDef;

/* VS MJPEG Frame Descriptor */
#pragma pack(1)
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;  // must be UVC_VS_INPUT_HEADER
  uint8_t bFrameIndex;
  uint8_t bmCapabilities;
  uint16_t wWidth;
  uint16_t wHeight;
  uint32_t dwMinBitRate;
  uint32_t dwMaxBitRate;
  uint32_t dwMaxVideoFrameBufferSize;
  uint32_t dwDefaultFrameInterval;
  uint8_t bFrameIntervalType;
  // dwFrameInterval*N is here
} VIDEO_MJPEGFrameDescTypeDef;
#pragma pack()

/* VS Uncompressed Format Typ Descriptor */
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;  // must be UVC_VS_INPUT_HEADER
  uint8_t bFormatIndex;
  uint8_t bNumFrameDescriptors;
  uint8_t guidFormat[16];
  uint8_t bBitsPerPixel;
  uint8_t bDefaultFrameIndex;
  uint8_t bAspectRatioX;
  uint8_t bAspectRatioY;
  uint8_t bmInterfaceFlags;
  uint8_t bCopyProtect;
} VIDEO_UncompFormatDescTypeDef;

/* VS Uncompressed Frame Descriptor */
#pragma pack(1)
typedef struct {
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;  // must be UVC_VS_INPUT_HEADER
  uint8_t bFrameIndex;
  uint8_t bmCapabilities;
  uint16_t wWidth;
  uint16_t wHeight;
  uint32_t dwMinBitRate;
  uint32_t dwMaxBitRate;
  uint32_t dwMaxVideoFrameBufferSize;
  uint32_t dwDefaultFrameInterval;
  uint8_t bFrameIntervalType;
  // dwFrameInterval*N is here
} VIDEO_UncompFrameDescTypeDef;
#pragma pack()

typedef struct uvc_frame_desc {
  uint8_t bLength;
  uint8_t bDescriptorType;
  /** Type of frame, such as JPEG frame or uncompressed frme */
  enum uvc_vs_desc_subtype bDescriptorSubtype;
  /** Index of the frame within the list of specs available for this format */
  uint8_t bFrameIndex;
  uint8_t bmCapabilities;
  /** Image width */
  uint16_t wWidth;
  /** Image height */
  uint16_t wHeight;
  /** Bitrate of corresponding stream at minimal frame rate */
  uint32_t dwMinBitRate;
  /** Bitrate of corresponding stream at maximal frame rate */
  uint32_t dwMaxBitRate;
  /** Maximum number of bytes for a video frame */
  uint32_t dwMaxVideoFrameBufferSize;
  /** Default frame interval (in 100ns units) */
  uint32_t dwDefaultFrameInterval;
  /** Minimum frame interval for continuous mode (100ns units) */
  uint32_t dwMinFrameInterval;
  /** Maximum frame interval for continuous mode (100ns units) */
  uint32_t dwMaxFrameInterval;
  /** Granularity of frame interval range for continuous mode (100ns) */
  uint32_t dwFrameIntervalStep;
  /** Frame intervals */
  uint8_t bFrameIntervalType;
  /** Available frame rates, zero-terminated (in 100ns units) */
  uint32_t *intervals;
} uvc_frame_desc_t;

/** Format descriptor
 *
 * A "format" determines a stream's image type (e.g., raw YUYV or JPEG)
 * and includes many "frame" configurations.
 */
typedef struct uvc_format_desc {
  uint8_t bLength;
  uint8_t bDescriptorType;
  /** Type of image stream, such as JPEG or uncompressed. */
  enum uvc_vs_desc_subtype bDescriptorSubtype;
  /** Identifier of this format within the VS interface's format list */
  uint8_t bFormatIndex;
  uint8_t bNumFrameDescriptors;
  /** Format specifier */
  union {
    uint8_t guidFormat[16];
    uint8_t fourccFormat[4];
  };
  /** Format-specific data */
  union {
    /** BPP for uncompressed stream */
    uint8_t bBitsPerPixel;
    /** Flags for JPEG stream */
    uint8_t bmFlags;
  };
  /** Default {uvc_frame_desc} to choose given this format */
  uint8_t bDefaultFrameIndex;
  uint8_t bAspectRatioX;
  uint8_t bAspectRatioY;
  uint8_t bmInterlaceFlags;
  uint8_t bCopyProtect;
  /** Available frame specifications for this format */
  uvc_frame_desc_t frame_descs[VIDEO_MAX_FORMAT_FRAME];
  uint8_t frame_descs_num;
} uvc_format_desc_t;

/* Class-Specific VC (Video Control) Interface Descriptor*/
typedef struct {
  VIDEO_HeaderDescTypeDef *HeaderDesc;
  VIDEO_ITDescTypeDef *InputTerminalDesc[VIDEO_MAX_NUM_IN_TERMINAL];
  VIDEO_OTDescTypeDef *OutputTerminalDesc[VIDEO_MAX_NUM_OUT_TERMINAL];
  VIDEO_FeatureDescTypeDef *FeatureUnitDesc[VIDEO_MAX_NUM_FEATURE_UNIT];
  VIDEO_SelectorDescTypeDef *SelectorUnitDesc[VIDEO_MAX_NUM_SELECTOR_UNIT];
} VIDEO_VCDescTypeDef;

/* Class-Specific VC (Video Control) Interface Descriptor*/
typedef struct {
  VIDEO_InHeaderDescTypeDef *InputHeader[VIDEO_MAX_NUM_IN_HEADER];

  // VIDEO_MJPEGFormatDescTypeDef *MJPEGFormat[VIDEO_MAX_MJPEG_FORMAT];
  // VIDEO_MJPEGFrameDescTypeDef *MJPEGFrame[VIDEO_MAX_MJPEG_FRAME_D];

  // VIDEO_UncompFormatDescTypeDef *UncompFormat[VIDEO_MAX_UNCOMP_FORMAT];
  // VIDEO_UncompFrameDescTypeDef *UncompFrame[VIDEO_MAX_MJPEG_FRAME_D];
  uvc_format_desc_t format_desc[VIDEO_MAX_FORMAT_DESC];
  uint8_t format_desc_num;
} VIDEO_VSDescTypeDef;

typedef struct {
  VIDEO_VCDescTypeDef cs_desc; /* Only one control descriptor*/
  VIDEO_VSDescTypeDef vs_desc;

  uint16_t ASNum;
  uint16_t InputTerminalNum;
  uint16_t OutputTerminalNum;
  uint16_t SelectorUnitNum;

  uint8_t InputHeaderNum;

} VIDEO_ClassSpecificDescTypedef;

//****************************************************************************

// UVC 1.0 uses only 26 first bytes
#pragma pack(1)
typedef struct {
  uint16_t bmHint;                    // 2
  uint8_t bFormatIndex;               // 3
  uint8_t bFrameIndex;                // 4
  uint32_t dwFrameInterval;           // 8
  uint16_t wKeyFrameRate;             // 10
  uint16_t wPFrameRate;               // 12
  uint16_t wCompQuality;              // 14
  uint16_t wCompWindowSize;           // 16
  uint16_t wDelay;                    // 18
  uint32_t dwMaxVideoFrameSize;       // 22
  uint32_t dwMaxPayloadTransferSize;  // 26
  uint32_t dwClockFrequency;
  uint8_t bmFramingInfo;
  uint8_t bPreferedVersion;
  uint8_t bMinVersion;
  uint8_t bMaxVersion;
  uint8_t bInterfaceNumber;  // custom
} VIDEO_ProbeTypedef;
#pragma pack()

typedef struct uvc_stream_ctrl {
  uint16_t bmHint;                    // 2
  uint8_t bFormatIndex;               // 3
  uint8_t bFrameIndex;                // 4
  uint32_t dwFrameInterval;           // 8
  uint16_t wKeyFrameRate;             // 10
  uint16_t wPFrameRate;               // 12
  uint16_t wCompQuality;              // 14
  uint16_t wCompWindowSize;           // 16
  uint16_t wDelay;                    // 18
  uint32_t dwMaxVideoFrameSize;       // 22
  uint32_t dwMaxPayloadTransferSize;  // 26
  uint32_t dwClockFrequency;
  uint8_t bmFramingInfo;
  uint8_t bPreferedVersion;
  uint8_t bMinVersion;
  uint8_t bMaxVersion;
  uint8_t bInterfaceNumber;  // custom
} uvc_stream_ctrl_t;

typedef struct _VIDEO_Process {
  VIDEO_ReqStateTypeDef req_state;
  VIDEO_ControlStateTypeDef control_state;
  VIDEO_StreamStateTypeDef steam_in_state;

  VIDEO_STREAMING_IN_HandleTypeDef stream_in[VIDEO_MAX_VIDEO_STD_INTERFACE];
  VIDEO_ClassSpecificDescTypedef class_desc;

  VIDEO_InterfaceStreamPropTypeDef camera;
  uint16_t mem[8];
  uint8_t temp_feature;
  uint32_t uvc_err_cnt;
} VIDEO_HandleTypeDef;

struct format_table_entry {
  enum uvc_frame_format format;
  uint8_t abstract_fmt;
  uint8_t guid[16];
  int children_count;
  enum uvc_frame_format *children;
};

/**
 * @}
 */

/** @defgroup USBH_VIDEO_CORE_Exported_Macros
 * @{
 */
/**
 * @}
 */

/** @defgroup USBH_VIDEO_CORE_Exported_Variables
 * @{
 */
extern USBH_ClassTypeDef VIDEO_Class;
#define USBH_VIDEO_CLASS &VIDEO_Class
/**
 * @}
 */

/** @defgroup USBH_VIDEO_CORE_Exported_FunctionsPrototype
 * @{
 */
USBH_StatusTypeDef USBH_VIDEO_SetFrequency(USBH_HandleTypeDef *phost, uint16_t sample_rate, uint8_t channel_num, uint8_t data_width);

USBH_StatusTypeDef USBH_VIDEO_Process(USBH_HandleTypeDef *phost);
USBH_StatusTypeDef uvc_get_stream_ctrl_format_size(USBH_HandleTypeDef *phost, uvc_stream_ctrl_t *ctrl, uvc_frame_format_t format, int width, int height,
                                                   int fps);
USBH_StatusTypeDef uvc_query_stream_ctrl(USBH_HandleTypeDef *phost, uvc_stream_ctrl_t *ctrl, uint8_t probe, enum uvc_req_code req);
USBH_StatusTypeDef uvc_stream_open_ctrl(USBH_HandleTypeDef *phost, uvc_stream_ctrl_t *ctrl);
USBH_StatusTypeDef uvc_probe_stream_ctrl(USBH_HandleTypeDef *phost, uvc_stream_ctrl_t *ctrl);

void uvc_stream_stop(USBH_HandleTypeDef *phost);

void uvc_stream_resume(USBH_HandleTypeDef *phost);

uint8_t _uvc_frame_format_matches_guid(enum uvc_frame_format fmt, uint8_t guid[16]);
enum uvc_frame_format uvc_frame_format_for_guid(uint8_t guid[16]);

typedef void (*PacketArrived)(uint8_t *packet, uint16_t packetLen, void *arg);
typedef struct {
  PacketArrived deliver_packet;
} USBVideoHandler;

enum {
  uvc_erro_no_pic = 0,
};

typedef void (*uvc_error_callback_t)(uint8_t msg);

void uvc_error_set_callback(uvc_error_callback_t callback);

void uvc_deinit(USBH_HandleTypeDef *phost);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* __USBH_VIDEO_H */
