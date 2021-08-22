/**
* @FILENAME uvc_video.h
* @BRIEF USB Video Class type definition & data structure
* Copyright (C) 2007 Anyka (Guangzhou) Software Technology Co., LTD
* @AUTHOR Tommy
* @DATE 2007-12-10
* @UPDATE 2007-12-11
* @VERSION 0.0.4
* @REF USB Video Class 1.0a Spec
*/

#ifndef _UVC_H_
#define _UVC_H_

#include "anyka_types.h"

//uvc desc type
#define INTERFACE_ASSOCIATION_DESC_TYPE                     0x0B


//  Video Interface Class Code
#define     CC_VIDEO                                        0x0E

//  Video Interface Subclass Codes
#define     SC_UNDEFINED                                    0x00
#define     SC_VIDEOCONTROL                                 0x01
#define     SC_VIDEOSTREAMING                               0x02
#define     SC_VIDEO_INTERFACE_COLLECTION                   0x03

//  Video Interface Protocol Codes
#define     PC_PROTOCOL_UNDEFINED                           0x00

//  Video Class-Specific Descriptor Types
#define     CS_UNDEFINED                                    0x20
#define     CS_DEVICE                                       0x21
#define     CS_CONFIGURATION                                0x22
#define     CS_STRING                                       0x23
#define     CS_INTERFACE                                    0x24
#define     CS_ENDPOINT                                     0x25

//  Video Class-Specific VC Interface Descriptor Subtypes
#define     VC_DESCRIPTOR_UNDEFINED                         0x00
#define     VC_HEADER                                       0x01
#define     VC_INPUT_TERMINAL                               0x02
#define     VC_OUTPUT_TERMINAL                              0x03
#define     VC_SELECTOR_UNIT                                0x04
#define     VC_PROCESSING_UNIT                              0x05
#define     VC_EXTENSION_UNIT                               0x06

//  Video Class-Specific VS Interface Descriptor Subtypes
#define     VS_UNDEFINED                                    0x00
#define     VS_INPUT_HEADER                                 0x01
#define     VS_OUTPUT_HEADER                                0x02
#define     VS_STILL_IMAGE_FRAME                            0x03
#define     VS_FORMAT_UNCOMPRESSED                          0x04
#define     VS_FRAME_UNCOMPRESSED                           0x05
#define     VS_FORMAT_MJPEG                                 0x06
#define     VS_FRAME_MJPEG                                  0x07
#define     VS_FORMAT_MPEG2TS                               0x0A
#define     VS_FORMAT_DV                                    0x0C
#define     VS_COLORFORMAT                                  0x0D
#define     VS_FORMAT_VENDOR                                0x0E
#define     VS_FRAME_VENDOR                                 0x0F
#define     VS_FORMAT_FRAME_BASED                           0x10
#define     VS_FRAME_FRAME_BASED                            0x11
#define     VS_FORMAT_STREAM_BASED                          0x12

//  Video Class-Specific Endpoint Descriptor Subtypes
#define     EP_UNDEFINED                                    0x00
#define     EP_GENERAL                                      0x01
#define     EP_ENDPOINT                                     0x02
#define     EP_INTERRUPT                                    0x03

//  Video Class-Specific Request Codes
#define     RC_UNDEFINED                                    0x00
#define     SET_CUR                                         0x01
#define     GET_CUR                                         0x81
#define     GET_MIN                                         0x82
#define     GET_MAX                                         0x83
#define     GET_RES                                         0x84
#define     GET_LEN                                         0x85
#define     GET_INFO                                        0x86
#define     GET_DEF                                         0x87

//  VideoControl Interface Control Selectors
#define     VC_CONTROL_UNDEFINED                            0x00
#define     VC_VIDEO_POWER_MODE_CONTROL                     0x01
#define     VC_REQUEST_ERROR_CODE_CONTROL                   0x02

//  Terminal Control Selectors
#define     TE_CONTROL_UNDEFINED                            0x00

//  Selector Unit Control Selectors
#define     SU_CONTROL_UNDEFINED                            0x00
#define     SU_INPUT_SELECT_CONTROL                         0x01
#define     CT_CONTROL_UNDEFINED                            0x00
#define     CT_SCANNING_MODE_CONTROL                        0x01
#define     CT_AE_MODE_CONTROL                              0x02
#define     CT_AE_PRIORITY_CONTROL                          0x03
#define     CT_EXPOSURE_TIME_ABSOLUTE_CONTROL               0x04
#define     CT_EXPOSURE_TIME_RELATIVE_CONTROL               0x05
#define     CT_FOCUS_ABSOLUTE_CONTROL                       0x06
#define     CT_FOCUS_RELATIVE_CONTROL                       0x07
#define     CT_FOCUS_AUTO_CONTROL                           0x08
#define     CT_IRIS_ABSOLUTE_CONTROL                        0x09
#define     CT_IRIS_RELATIVE_CONTROL                        0x0A
#define     CT_ZOOM_ABSOLUTE_CONTROL                        0x0B
#define     CT_ZOOM_RELATIVE_CONTROL                        0x0C
#define     CT_PANTILT_ABSOLUTE_CONTROL                     0x0D
#define     CT_PANTILT_RELATIVE_CONTROL                     0x0E
#define     CT_ROLL_ABSOLUTE_CONTROL                        0x0F
#define     CT_ROLL_RELATIVE_CONTROL                        0x10
#define     CT_PRIVACY_CONTROL                              0x11

//  Processing Unit Control Selectors
#define     PU_CONTROL_UNDEFINED                            0x00
#define     PU_BACKLIGHT_COMPENSATION_CONTROL               0x01
#define     PU_BRIGHTNESS_CONTROL                           0x02
#define     PU_CONTRAST_CONTROL                             0x03
#define     PU_GAIN_CONTROL                                 0x04
#define     PU_POWER_LINE_FREQUENCY_CONTROL                 0x05
#define     PU_HUE_CONTROL                                  0x06
#define     PU_SATURATION_CONTROL                           0x07
#define     PU_SHARPNESS_CONTROL                            0x08
#define     PU_GAMMA_CONTROL                                0x09
#define     PU_WHITE_BALANCE_TEMPERATURE_CONTROL            0x0A
#define     PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL       0x0B
#define     PU_WHITE_BALANCE_COMPONENT_CONTROL              0x0C
#define     PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL         0x0D
#define     PU_DIGITAL_MULTIPLIER_CONTROL                   0x0E
#define     PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL             0x0F
#define     PU_HUE_AUTO_CONTROL                             0x10
#define     PU_ANALOG_VIDEO_STANDARD_CONTROL                0x11
#define     PU_ANALOG_LOCK_STATUS_CONTROL                   0x12

//  Extension Unit Control Selectors
#define     XU_CONTROL_UNDEFINED                            0x00

//  VideoStreaming Interface Control Selectors
#define     VS_CONTROL_UNDEFINED                            0x00
#define     VS_PROBE_CONTROL                                0x01
#define     VS_COMMIT_CONTROL                               0x02
#define     VS_STILL_PROBE_CONTROL                          0x03
#define     VS_STILL_COMMIT_CONTROL                         0x04
#define     VS_STILL_IMAGE_TRIGGER_CONTROL                  0x05
#define     VS_STREAM_ERROR_CODE_CONTROL                    0x06
#define     VS_GENERATE_KEY_FRAME_CONTROL                   0x07
#define     VS_UPDATE_FRAME_SEGMENT_CONTROL                 0x08
#define     VS_SYNCH_DELAY_CONTROL                          0x09

//  USB Terminal Types
#define     TT_VENDOR_SPECIFIC                              0x0100
#define     TT_STREAMING                                    0x0101

//  Input Terminal Types
#define     ITT_VENDOR_SPECIFIC                             0x0200
#define     ITT_CAMERA                                      0x0201
#define     ITT_MEDIA_TRANSPORT_INPUT                       0x0202

//  Output Terminal Types
#define     OTT_VENDOR_SPECIFIC                             0x0300
#define     OTT_DISPLAY                                     0x0301
#define     OTT_MEDIA_TRANSPORT_OUTPUT                      0x0302

//  External Terminal Types
#define     EXTERNAL_VENDOR_SPECIFIC                        0x0400
#define     COMPOSITE_CONNECTOR                             0x0401
#define     SVIDEO_CONNECTOR                                0x0402
#define     COMPONENT_CONNECTOR                             0x0403

#define UVC_GUID_FORMAT_YUY2  {0x59, 0x55, 0x59, 0x32, 0x00, 0x00, 0x10, 0x00,\
                              0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}
#define UVC_GUID_FORMAT_NV12  {0x4e, 0x56, 0x31, 0x32, 0x00, 0x00, 0x10, 0x00,\
                              0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}



//  Standard Video Interface Collection IAD
//  Table 3-1
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_INTERFACE_ASSOCIATION_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bFirstInterface;
    unsigned char    bInterfaceCount;
    unsigned char    bFunctionClass;
    unsigned char    bFunctionSubClass;
    unsigned char    bFunctionProtocol;
    unsigned char    iFunction;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif 
T_UVC_INTERFACE_ASSOCIATION_DESCRIPTOR, *T_pUVC_INTERFACE_ASSOCIATION_DESCRIPTOR;

//  Class-specific VC Interface Header Descriptor
//  Table 3-3
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_VIDEOCONTROL_INTERFACE_HEADER_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubType;
    unsigned short    bcdVDC;
    unsigned short    wTotalLength;
    unsigned long   dwClockFrequency;
    unsigned char    bInCollection;
    unsigned char    baInterfaceNr1;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif 
T_UVC_VIDEOCONTROL_INTERFACE_HEADER_DESCRIPTOR, *T_pUVC_VIDEOCONTROL_INTERFACE_HEADER_DESCRIPTOR;

//  Input Terminal Descriptor
//  Table 3-4
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_INPUT_TERMINAL_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubtype;
    unsigned char    bTerminalID;
    unsigned short    wTerminalType;
    unsigned char    bAssocTerminal;
    unsigned char    iTerminal;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif 
T_UVC_INPUT_TERMINAL_DESCRIPTOR, *T_pUVC_INPUT_TERMINAL_DESCRIPTOR;

//  Output Terminal Descriptor
//  Table 3-5
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_OUTPUT_TERMINAL_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubtype;
    unsigned char    bTerminalID;
    unsigned short    wTerminalType;
    unsigned char    bAssocTerminal;
    unsigned char    bSourceID;
    unsigned char    iTerminal;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif 
T_UVC_OUTPUT_TERMINAL_DESCRIPTOR, *T_pUVC_OUTPUT_TERMINAL_DESCRIPTOR;

//  Camera Terminal Descriptor
//  Table 3-6
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_CAMERA_TERMINAL_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubtype;
    unsigned char    bTerminalID;
    unsigned short    wTerminalType;
    unsigned char    bAssocTerminal;
    unsigned char    iTerminal;
    unsigned short    wObjectiveFocalLengthMin;
    unsigned short    wObjectiveFocalLengthMax;
    unsigned short    wOcularFocalLength;
    unsigned char    bControlSize;
    unsigned short    bmControls;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_CAMERA_TERMINAL_DESCRIPTOR, *T_pUVC_CAMERA_TERMINAL_DESCRIPTOR;

//  Selector Unit Descriptor
//  Table 3-7
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_SELECTOR_UNIT_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubtype;
    unsigned char    bUnitID;
    unsigned char    bNrInPins;
    unsigned char    baSourceID1;
    unsigned char    baSourceID2;
    unsigned char    iSelector;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_SELECTOR_UNIT_DESCRIPTOR, *T_pUVC_SELECTOR_UNIT_DESCRIPTOR;

//  Processing Unit Descriptor
//  Table 3-8
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_PROCESSING_UNIT_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubtype;
    unsigned char    bUnitID;
    unsigned char    bSourceID;
    unsigned short    wMaxMultiplier;
    unsigned char    bControlSize;
    unsigned short    bmControls;
    unsigned char    iProcessing;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_PROCESSING_UNIT_DESCRIPTOR, *T_pUVC_PROCESSING_UNIT_DESCRIPTOR;

//  Extension Unit Descriptor
//  Table 3-9
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_EXTENSION_UNIT_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubtype;
    unsigned char    bUnitID;
    unsigned char    guidExtensionCode[16];
    unsigned char    bNumControls;
    unsigned char    bNrInPins;
    unsigned char    baSourceID1;
    unsigned char    baSourceID2;
    unsigned char    bControlSize;
    unsigned char    bmControls;
    unsigned char    iExtension;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_EXTENSION_UNIT_DESCRIPTOR, *T_pUVC_EXTENSION_UNIT_DESCRIPTOR;

//  Class-specific VS Interface Input Header Descriptor
//  Table 3-13
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubtype;
    unsigned char    bNumFormats;
    unsigned short    wTotalLength;
    unsigned char    bEndpointAddress;
    unsigned char    bmInfo;
    unsigned char    bTerminalLink;
    unsigned char    bStillCaptureMethod;
    unsigned char    bTriggerSupport;
    unsigned char    bTriggerUsage;
    unsigned char    bControlSize;
    unsigned char    bmaControls;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR, *T_pUVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR;

#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_STILL_IMAGE_FRAME_DESCRIPTOR
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype;
    unsigned char bEndpointAddress;
    unsigned char bNumImageSizePatterns;
    unsigned short wWidth1;
    unsigned short wHeight1;
    unsigned short wWidth2;
    unsigned short wHeight2;
    unsigned short wWidth3;
    unsigned short wHeight3;
    unsigned short wWidth4;
    unsigned short wHeight4;
    unsigned char  bNumCompressionPattern;
}
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_STILL_IMAGE_FRAME_DESCRIPTOR,*T_pUVC_STILL_IMAGE_FRAME_DESCRIPTOR;

#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_COLOR_MATCHING_DESCRIPTOR
{
    unsigned char bLength;
    unsigned char bDescriptorType;
    unsigned char bDescriptorSubtype;
    unsigned char bColorPrimaries;
    unsigned char bTransferCharacteristics;
    unsigned char bMatrixCoefficients;
}
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_COLOR_MATCHING_DESCRIPTOR,*T_pUVC_COLOR_MATCHING_DESCRIPTOR;


//  Class-specific VS Format Descriptor for Motion-JPEG
//  Table 3-1 in Payload MJPEG
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_MJPEG_VIDEO_FORMAT_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubtype;
    unsigned char    bFormatIndex;
    unsigned char    bNumFrameDescriptors;
    unsigned char    bmFlags;
    unsigned char    bDefaultFrameIndex;
    unsigned char    bAspectRatioX;
    unsigned char    bAspectRatioY;
    unsigned char    bmInterlaceFlags;
    unsigned char    bCopyProtect;
}
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_MJPEG_VIDEO_FORMAT_DESCRIPTOR, *T_pUVC_MJPEG_VIDEO_FORMAT_DESCRIPTOR;

//  Class-specific VS Format Descriptor for Uncompressed
//  Table 3-1 in Payload Uncompressed
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_UNCOMPRESSED_VIDEO_FORMAT_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubtype;
    unsigned char    bFormatIndex;
    unsigned char    bNumFrameDescriptors;
    unsigned char    guidFormat[16];
    unsigned char    bBitsPerPixel;
    unsigned char    bDefaultFrameIndex;
    unsigned char    bAspectRatioX;
    unsigned char    bAspectRatioY;
    unsigned char    bmInterlaceFlags;
    unsigned char    bCopyProtect;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_UNCOMPRESSED_VIDEO_FORMAT_DESCRIPTOR, *T_pUVC_UNCOMPRESSED_VIDEO_FORMAT_DESCRIPTOR;

//  Class-specific VS Frame Descriptor
//  Table 3-2 in Payload MJPEG
//  Table 3-2 in Payload Uncompressed
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_VIDEOSTREAM_FRAME_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubtype;
    unsigned char    bFrameIndex;
    unsigned char    bmCapabilities;
    unsigned short    wWidth;
    unsigned short    wHeight;
    unsigned long   dwMinBitRate;
    unsigned long   dwMaxBitRate;
    unsigned long   dwMaxVideoFrameBufSize;
    unsigned long   dwDefaultFrameInterval;
    unsigned char    bFrameIntervalType;
    unsigned long   dwMinFrameInterval;
    unsigned long   dwMaxFrameInterval;
    unsigned long   dwFrameIntervalStep;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR, *T_pUVC_VIDEOSTREAM_FRAME_DESCRIPTOR;

//  Class-specific VC Interrupt Endpoint Descriptor
//  Table 3.8.2.2
#ifdef __CC_ARM
__packed
#endif
typedef struct _UVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR
{
    unsigned char    bLength;
    unsigned char    bDescriptorType;
    unsigned char    bDescriptorSubType;
    unsigned short    wMaxTransferSize;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_UVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR, *T_pUVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR;

//  Color Matching Descriptor
#ifdef __CC_ARM
__packed
#endif
typedef struct _USB_VIDEO_CMD
{
    //  6
    unsigned char    bLength;
    //  CS_INTERFACE type
    unsigned char    bDescriptorType;
    //  VS_COLORFORMAT
    unsigned char    bDescriptorSubtype;
    //  This defines the color primaries and the reference white.
    //  0: Unspecified (Image characteristics unknown)
    //  1: BT.709, sRGB (default)
    //  2: BT.470-2 (M)3: BT.470-2 (B, G)
    //  4: SMPTE 170M
    //  5: SMPTE 240M
    //  6-255: Reserved
    unsigned char    bColorPrimaries;
    //  This field defines the optoelectronic transfer characteristic of the source picture also called the gamma function.
    //  0: Unspecified (Image characteristics unknown)
    //  1: BT.709 (default)
    //  2: BT.470-2 M
    //  3: BT.470-2 B, G
    //  4: SMPTE 170M
    //  5: SMPTE 240M
    //  6: Linear (V = Lc)
    //  7: sRGB (very similar to BT.709)
    //  8-255: Reserved
    unsigned char    bTransferCharacteristics;
    //  Matrix used to compute luma and chroma values from the color primaries.
    //  0: Unspecified (Image characteristics unknown)
    //  1: BT. 709
    //  2: FCC
    //  3: BT.470-2 B, G
    //  4: SMPTE 170M (BT.601, default)
    //  5: SMPTE 240M
    //  6-255: Reserved
    unsigned char    bMatrixCoefficients;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
USB_VIDEO_CMD;

#ifdef __CC_ARM
__packed
#endif
typedef struct _VIDEO_PROBE_COMMIT_CONTROLS
{
    unsigned short    bmHint;
    unsigned char    bFormatIndex;
    unsigned char    bFrameIndex;
    unsigned long   dwFrameInterval;
    unsigned short    wKeyFrameRate;
    unsigned short    wPFrameRate;
    unsigned short    wCompQuality;
    unsigned short    wCompWindowSize;
    unsigned short    wDelay;
    unsigned long   dwMaxVideoFrameSize;
    unsigned long   dwMaxPayloadTransferSize;
} 
#ifdef __GNUC__
__attribute__((packed))
#endif  
T_VIDEO_PROBE_COMMIT_CONTROLS,*T_pVIDEO_PROBE_COMMIT_CONTROLS;


#endif  //  _UVC_H_
