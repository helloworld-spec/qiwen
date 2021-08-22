/**
 * @filename hal_s_uvc.c
 * @brief: AK880x how to use slave uvc.
 *
 * This file describe frameworks of usb slave uvc driver.
 * Copyright (C) 2006 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author  Huang Xin
 * @date    2010-08-05
 * @version 1.0
 */
#include <stdio.h>
#include "usb_slave_drv.h"
#include "hal_usb_s_std.h"
#include "usb_common.h"
#include "hal_s_uvc.h"
#include "interrupt.h"
#include "akos_api.h"
#include "drv_api.h"
#include "drv_module.h"

#define FREQ_REQUST_ASIC     140000000
static T_hFreq freq_handle = FREQ_INVALID_HANDLE;

volatile  bool  g_bUvcSending = false;

static T_pUVC_DEV s_pUvcDev = NULL;
static unsigned char s_ConfigData[500] = {0};

static void uvc_dev_msg_ctrl_proc(unsigned long* pMsg,unsigned long len);
static void uvc_dev_msg_send_proc(unsigned long* pMsg,unsigned long len);
static void vc_ctrl_handle(unsigned char unit, unsigned char cs , unsigned long cur_val1,unsigned long cur_val2);
static void vs_ctrl_handle(unsigned char unit, unsigned char cs , unsigned long cur_val1,unsigned long cur_val2);
static bool uvc_enable();
static bool uvc_disable();
static void uvc_reset(unsigned long mode);
static void uvc_suspend();
static void uvc_resume();
static void uvc_configok();
static void uvc_send_finish();
static bool uvc_class_callback(T_CONTROL_TRANS *pTrans);
static unsigned char *uvc_get_dev_qualifier_desc(unsigned long *count);
static unsigned char *uvc_get_dev_desc(unsigned long *count);
static unsigned char *uvc_get_cfg_desc(unsigned long *count);
static unsigned char *uvc_get_other_speed_cfg_desc(unsigned long *count);
static unsigned char *uvc_get_str_desc(unsigned char index, unsigned long *count);

const static T_UVC_FRAME_RES s_UvcFramesRes[UVC_FRAME_NUM] =
{
    { 1280, 720 },
    { 640, 480 },
    { 320, 240 },
    { 160, 120 },
};


const static T_USB_DEVICE_DESCRIPTOR s_UvcDeviceDesc =
{
    0x12,                           ///< Descriptor size is 18 bytes
    DEVICE_DESC_TYPE,               ///< DEVICE Descriptor Type  
    0x0200,                         ///< 0x02 USB Specification Release Number
    0xEF,                           ///< The device belongs to the Miscellaneous Device Class  
    0x02,                           ///< The device belongs to the Common Class Subclass  
    0x01,                           ///< The device uses the Interface Association Descriptor Protocol  
    EP0_MAX_PAK_SIZE,                ///< Maximum packet size for endpoint zero is 64  
    UVC_VID,                        ///< Vendor ID
    UVC_PID,                        ///< Product ID
    UVC_BCD,                        ///< Device release number in binary-coded
    0x01,                           ///< The manufacturer string descriptor index is 1  
    0x02,                           ///< The product string descriptor index is 2
    0x03,                           ///< The serial number string descriptor index is 3 
    0x01                            ///< The device has 1 possible configurations 
};

const static T_USB_DEVICE_QUALIFIER_DESCRIPTOR s_UvcDeviceQualifierDesc=
{
    sizeof(T_USB_DEVICE_QUALIFIER_DESCRIPTOR),      ///<Descriptor size is 10 bytes  
    DEVICE_QUALIFIER_DESC_TYPE,                     ///<DEVICE_QUALIFIER Descriptor Type  
    0x0200,                                         ///<  USB Specification version 2.00  
    0xEF,                                           ///<The device belongs to the Miscellaneous Device Class  
    0x02,                                           ///<The device belongs to the Common Class Subclass
    0x01,                                           ///<The device uses the Interface Association Descriptor Protocol  
    EP0_MAX_PAK_SIZE,                               ///<Maximum packet size for endpoint zero is 64  
    0x01,                                           ///<The device has 1 possible other-speed configurations  
    0                                               ///<Reserved for future use  
};

static T_USB_CONFIGURATION_DESCRIPTOR s_UvcConfigDesc =
{
    0x9,                        ///<Descriptor size is 9 bytes  
    CONFIG_DESC_TYPE,           ///<CONFIGURATION Descriptor Type  
    0,                          ///<The total length of data for this configuration is define when init. This includes the combined length of all the descriptors
    UVC_INTERFACE_NUM,          ///<This configuration supports 2 interfaces 
    0x01,                       ///< The value 1 should be used to select this configuration  
    0x00,                       ///<The device doesn't have the string descriptor describing this configuration  
    0x80,                       ///<Configuration characteristics : Bit 7: Reserved (set to one) 1 Bit 6: Self-powered 0 Bit 5: Remote Wakeup 0  
    0xFA                        ///<Maximum power consumption of the device in this configuration is 500 mA  
};

static T_USB_OTHER_SPEED_CONFIGURATION_DESCRIPTOR s_UvcOtherSpeedConfigDesc=
{
    0x9,                                        ///<Descriptor size is 9 bytes  
    OTHER_SPEED_CONFIGURATION_DESC_TYPE,        ///<CONFIGURATION Descriptor Type  
    0,                                          ///<The total length of data for this configuration is define when init. This includes the combined length of all the descriptors
    UVC_INTERFACE_NUM,                          ///<This configuration supports 2 interfaces 
    0x01,                                       ///< The value 1 should be used to select this configuration  
    0x00,                                       ///<The device doesn't have the string descriptor describing this configuration  
    0x80,                                       ///<Configuration characteristics : Bit 7: Reserved (set to one) 1 Bit 6: Self-powered 0 Bit 5: Remote Wakeup 0  
    0xFA                                        ///<Maximum power consumption of the device in this configuration is 500 mA  
};


const static T_UVC_INTERFACE_ASSOCIATION_DESCRIPTOR s_UvcInterfaceAssociationDesc=
{
    sizeof(T_UVC_INTERFACE_ASSOCIATION_DESCRIPTOR),             ///<  Descriptor size is 8 bytes
    INTERFACE_ASSOCIATION_DESC_TYPE,                            ///<INTERFACE_ASSOCIATION Descriptor Type  
    0x00,                                                       ///< The first interface number associated with this function is 0  
    UVC_INTERFACE_NUM,                                          ///<The number of contiguous interfaces associated with this function is 2  
    CC_VIDEO,                                                   ///<The function belongs to the Video Class  
    SC_VIDEO_INTERFACE_COLLECTION,                              ///<The function belongs to the SC_VIDEO_INTERFACE_COLLECTION Subclass 
    PC_PROTOCOL_UNDEFINED,                                      ///<The function uses the PC_PROTOCOL_UNDEFINED Protocol 
    0x02                                                        ///<The Function string descriptor index is 2  
};

const static T_USB_INTERFACE_DESCRIPTOR s_UvcVciDesc=
{
    0x09,                                                   ///<Descriptor size is 9 bytes  
    IF_DESC_TYPE,                                           ///<INTERFACE Descriptor Type
    UVC_VCI_ID,                                                   ///<The number of this interface is 0. 
    0x00,                                                   ///<The value used to select the alternate setting for this interface is 0  
    0x01,                                                   ///<The number of endpoints used by this interface is 1 (excluding endpoint zero)  
    CC_VIDEO,                                               ///<The interface implements the Video class  
    SC_VIDEOCONTROL,                                        ///<The interface implements the SC_VIDEOCONTROL Subclass  
    PC_PROTOCOL_UNDEFINED,                                  ///<The interface uses the PC_PROTOCOL_UNDEFINED Protocol  
    0x02                                                    ///<The interface string descriptor index is 2  
};

const static T_UVC_VIDEOCONTROL_INTERFACE_HEADER_DESCRIPTOR s_UvcVideoCtrlInterfaceHeaderDesc=
{
    0x0D,                                                       ///< Descriptor size is 13 bytes  
    CS_INTERFACE,                                               ///<CS_INTERFACE Descriptor Type  
    VC_HEADER,                                                  ///<VC_HEADER descriptor subtype  
    0x0100,                                                     ///< Video Device Class Specification release 1.00  
    sizeof(T_UVC_VIDEOCONTROL_INTERFACE_HEADER_DESCRIPTOR) +    
    sizeof(T_UVC_CAMERA_TERMINAL_DESCRIPTOR) +
    sizeof(T_UVC_INPUT_TERMINAL_DESCRIPTOR) +
    sizeof(T_UVC_OUTPUT_TERMINAL_DESCRIPTOR) +
    sizeof(T_UVC_SELECTOR_UNIT_DESCRIPTOR) +
    sizeof(T_UVC_PROCESSING_UNIT_DESCRIPTOR),                   ///<The total bytes of data for the Class Specific VideoControl interface descriptor is 77
    0x112a880,                                                 ///The device clock frequency is 6000000 Hz  0x005B8D80 *3
    0x01,                                                       ///<The number of VideoStreaming interfaces in the Video Interface Collection to which this VideoConrol interface belongs is : 1  
    0x01                                                        ///<VideoStreaming interface number 1 in the collection  
};

const static T_UVC_CAMERA_TERMINAL_DESCRIPTOR s_UvcCameraTerminalDesc = 
{
    sizeof(T_UVC_CAMERA_TERMINAL_DESCRIPTOR),                     ///<Descriptor size is 17 bytes  
    CS_INTERFACE,                                                 ///<CS_INTERFACE Descriptor Type  
    VC_INPUT_TERMINAL,                                            ///<VC_INPUT_TERMINAL
    UVC_CT_ID,                                                         ///< Unique Terminal Identifier is 1  
    ITT_CAMERA,                                                   ///<  ITT_CAMERA type. This terminal is a camera terminal representing the CCD sensor.
    0x00,                                                         ///<This Input Terminal is associated with Output Terminal ID 0.  
    0x00,                                                         ///<The index of the Input Terminal string descriptor is 0  
    0x0000,                                                       ///<  No optical zoom supported
    0x0000,                                                       ///<  No optical zoom supported
    0x0000,                                                       ///<  No optical zoom supported
    0x02,                                                         ///<  The size of the bmControls is 2 bytes (this terminal doesn't implement any controls).
    0x0000                                                        ///<  No controls are supported.
};
const static T_UVC_INPUT_TERMINAL_DESCRIPTOR s_UvcInputTerminalDesc =
{
    sizeof(T_UVC_INPUT_TERMINAL_DESCRIPTOR),                 ///<  Size of this descriptor, in bytes.
    CS_INTERFACE,                                            ///<  CS_INTERFACE
    VC_INPUT_TERMINAL,                                       ///<  INPUT_TERMINAL subtype
    UVC_IT_ID,                                                    ///<  ID of this input terminal
    COMPOSITE_CONNECTOR,                                     ///<  COMPOSITE_CONNECTOR type. This terminal is the composite connector.
    0x00,                                                    ///<  No association
    0x00                                                     ///<  Unused
};

const static T_UVC_OUTPUT_TERMINAL_DESCRIPTOR s_UvcOutputTerminalDesc = 
{
    sizeof(T_UVC_OUTPUT_TERMINAL_DESCRIPTOR),                       ///<Descriptor size is 9 bytes  
    CS_INTERFACE,                                                   ///<CS_INTERFACE Descriptor Type  
    VC_OUTPUT_TERMINAL,                                             ///<  VC_OUTPUT_TERMINAL
    UVC_OT_ID,                                                           ///<  Unique Terminal Identifier is 2  
    TT_STREAMING,                                                   ///<  TT_STREAMING type. This terminal is a USB streaming terminal.
    0x00,                                                           ///<  This Output Terminal is associated with Input Terminal ID 0  
    0x05,                                                           ///<  The input pin of this unit is connected to the output pin of unit 5.
    0x00                

};
const static T_UVC_SELECTOR_UNIT_DESCRIPTOR s_UvcSelectUnitDesc =   
{
    sizeof(T_UVC_SELECTOR_UNIT_DESCRIPTOR),                         ///<  Size of this descriptor, in bytes.
    CS_INTERFACE,                                                   ///<  CS_INTERFACE descriptor type
    VC_SELECTOR_UNIT,                                               ///<  VC_SELECTOR_UNIT descriptor subtype
    UVC_SU_ID,                                                           ///<  ID of this unit
    0x02,                                                           ///<  Number of input pins
    0x01,                                                           ///< Input 1 of this unit is connected to unit ID 0x01 每 the CMOS sensor.
    0x02,                                                           ///<  Input 2 of this unit is connected to unit ID 0x02 每 the composite connector.
    0x00                                                            ///<  Unused
};

static T_UVC_PROCESSING_UNIT_DESCRIPTOR s_UvcProcessUnitDesc = 
{
   sizeof(T_UVC_PROCESSING_UNIT_DESCRIPTOR),                        ///<  Size of this descriptor, in bytes.
   CS_INTERFACE,                                                    ///<  CS_INTERFACE
   VC_PROCESSING_UNIT,                                              ///<  VC_PROCESSING_UNIT
   UVC_PU_ID,                                                            ///<  ID of this unit
   0x04,                                                            ///< This input pin of this unit is connected to the output pin of unit with ID 0x04.
   0x0000,                                                          ///<  Optical Zoom Not Supported  
   0x02,                                                            ///<  Size of the bmControls field, in bytes.
   0x0000,                                                          ///<  Control Setting
   0x00                                                             ///<  Unused
};
const static T_USB_ENDPOINT_DESCRIPTOR  s_UvcEp1Desc =
{
    sizeof(T_USB_ENDPOINT_DESCRIPTOR),    ///<  Size of this descriptor, in bytes.
    EP_DESC_TYPE,                         ///<  ENDPOINT descriptor
    (ENDPOINT_DIR_IN + EP1_INDEX),        ///<  IN endpoint 1
    0x03,                                 ///<  Interrupt transfer type
    0x0010,                               ///< Maximum packet size for this endpoint is 16 Bytes. If Hi-Speed, 0 additional transactions per frame  
    0x6                                   ///< The polling interval value is every 6 Frames. If Hi-Speed, every 32 uFrames  
};
const static T_UVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR s_UvcVideoCtrlIntEpDesc =
{
     
    sizeof(T_UVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR),   ///<  Size of this descriptor, in bytes.
    CS_ENDPOINT,                                                ///<  CS_ENDPOINT descriptor
    EP_INTERRUPT,                                               ///<  EP_INTERRUP
    0x0010                                                      ///< The maximum interrupt structure size this endpoint is capable of sending is 16  
};

const static T_USB_INTERFACE_DESCRIPTOR s_UvcVsiDesc =
{
    sizeof(T_USB_INTERFACE_DESCRIPTOR),     ///<  Size of this descriptor, in bytes.
    IF_DESC_TYPE,                           ///<  INTERFACE descriptor type
    UVC_VSI_ID,                             ///<The number of this interface is 1.  
    0x00,                                   ///<  Index of this alternate setting
    0x01,                                   ///<  The number of endpoints used by this interface is 1
    CC_VIDEO,                               ///<  CC_VIDEO
    SC_VIDEOSTREAMING,                      ///<  SC_VIDEOSTREAMING
    PC_PROTOCOL_UNDEFINED,                  ///<  PC_PROTOCOL_UNDEFINED
    0x00                                    ///<  Unused
};

static T_UVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR s_UvcVideoStreamInterfaceInputHeaderDesc =
{   
    sizeof(T_UVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR),    ///<  Size of this descriptor, in bytes.
    CS_INTERFACE,                                                   ///<  CS_INTERFACE
    VS_INPUT_HEADER,                                                ///<VS_INPUT_HEADER descriptor subtype  
    0x01,                                                           ///<  One format descriptor follows.
    0,                                                              ///<Total number of bytes returned for the class-specific VideoStreaming interface descriptors including this header descriptor is define in init  
    (ENDPOINT_DIR_IN + EP2_INDEX),                                  ///<  Address of the isochronous endpoint used for video data
    0x00,                                                           ///<  Video Streaming interface Capabilities: the video endpoint of this interface is connected is 0  
    0x03,                                                           ///<  This VideoStreaming interface supplies terminal ID 3 (Output Terminal).
    0x01,                                                           ///<  Device supports still image capture method 1.
    0x00,                                                           ///<  Hardware trigger not supported for still image capture
    0x00,                                                           ///<  Hardware trigger should initiate a still image capture.
    0x01,                                                           ///< Size of the bmaControls field
    0x00                                                            ///<  No VideoStreaming specific controls are supported.
};
const static T_UVC_MJPEG_VIDEO_FORMAT_DESCRIPTOR s_UvcMjpegFormatDesc =
{
   sizeof(T_UVC_MJPEG_VIDEO_FORMAT_DESCRIPTOR), ///<  Size of this descriptor, in bytes.
   CS_INTERFACE,                                ///<  CS_INTERFACE
   VS_FORMAT_MJPEG,                             ///<  VS_FORMAT_MJPEG
   0x01,                                        ///<  format index
   UVC_FRAME_NUM,                               ///<  Four frame descriptor for this format follows.
   0x01,                                        ///<  Uses fixed size samples.
   0x01,                                        ///<  Default frame index is 1.
   0x00,                                        ///<  Non-interlaced stream 每 not required.
   0x00,                                        ///<  Non-interlaced stream 每 not required.
   0x00,                                        ///<  Non-interlaced stream
   0x00                                         ///<  No restrictions imposed on the duplication of this video stream.
};

const static T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR s_UvcMjpegFrame[UVC_FRAME_NUM]=
{
    //frame1:1280*720
    {
        sizeof(T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR),        ///<Size of frame descriptor, in bytes.
        CS_INTERFACE,                                      ///<  CS_INTERFACE type
        VS_FRAME_MJPEG,                                    ///<mjpeg frame sub type
        0x01,                                              ///<frame index
        0x01,                                              ///<capabilities
        1280,                                               ///<width
        720,                                               ///<height
        1280*720*36,                                        ///<min bit rate
        1280*720*36,                                        ///<max bit rate
        1280*720*3/2,                                       ///<max frame buffer size
        0x00051615,                                        ///<Default frame interval is 333333x100ns (30fps).
        0x00,                                              ///<  Continuous frame interval
        0x00051615,                                        ///<  Minimum frame interval is 333333x100ns (30fps)
        0x00051615,                                        ///<  Maximum frame interval is 333333x100ns(30fps).
        0x00000000,                                        ///<  No frame interval step supported.
    },

    //frame1:640x480
    {
        sizeof(T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR),        ///<Size of frame descriptor, in bytes.
        CS_INTERFACE,                                      ///<  CS_INTERFACE type
        VS_FRAME_MJPEG,                                    ///<mjpeg frame sub type
        0x01,                                              ///<frame index
        0x01,                                              ///<capabilities
        640,                                               ///<width
        480,                                               ///<height
        640*480*36,                                        ///<min bit rate
        640*480*36,                                        ///<max bit rate
        640*480*3/2,                                       ///<max frame buffer size
        0x00051615,                                        ///<Default frame interval is 333333x100ns (30fps).
        0x00,                                              ///<  Continuous frame interval
        0x00051615,                                        ///<  Minimum frame interval is 333333x100ns (30fps)
        0x00051615,                                        ///<  Maximum frame interval is 333333x100ns(30fps).
        0x00000000,                                        ///<  No frame interval step supported.
    },
    //frame2:320x240
    {
        sizeof(T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR),        ///<Size of frame descriptor, in bytes.
        CS_INTERFACE,                                      ///<  CS_INTERFACE type
        VS_FRAME_MJPEG,                                    ///<mjpeg frame sub type
        0x02,                                              ///<frame index
        0x01,                                              ///<capabilities
        320,                                               ///<width
        240,                                               ///<height
        320*240*36,                                        ///<min bit rate
        320*240*36,                                        ///<max bit rate
        320*240*3/2,                                       ///<max frame buffer size
        0x00051615,                                        ///<Default frame interval is 333333x100ns (30fps).
        0x00,                                              ///<  Continuous frame interval
        0x00051615,                                        ///<  Minimum frame interval is 333333x100ns (30fps)
        0x00051615,                                        ///<  Maximum frame interval is 333333x100ns(30fps).
        0x00000000,                                        ///<  No frame interval step supported.
    },
    //frame4:160x120
    {
        sizeof(T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR),        ///<Size of frame descriptor, in bytes.
        CS_INTERFACE,                                      ///<  CS_INTERFACE type
        VS_FRAME_MJPEG,                                    ///<mjpeg frame sub type
        0x04,                                              ///<frame index
        0x01,                                              ///<capabilities
        160,                                               ///<width
        120,                                               ///<height
        160*120*36,                                        ///<min bit rate
        160*120*36,                                        ///<max bit rate
        160*120*3/2,                                       ///<max frame buffer size
        0x00051615,                                        ///<Default frame interval is 333333x100ns (30fps).
        0x00,                                              ///<  Continuous frame interval
        0x00051615,                                        ///<  Minimum frame interval is 333333x100ns (30fps)
        0x00051615,                                        ///<  Maximum frame interval is 333333x100ns(30fps).
        0x00000000,                                        ///<  No frame interval step supported.
    },
};
const static T_UVC_UNCOMPRESSED_VIDEO_FORMAT_DESCRIPTOR s_UvcUncompressedFormatDesc =
{
   sizeof(T_UVC_UNCOMPRESSED_VIDEO_FORMAT_DESCRIPTOR), ///<  Size of this descriptor, in bytes.
   CS_INTERFACE,                                ///<  CS_INTERFACE
   VS_FORMAT_UNCOMPRESSED,                      ///<  VS_FORMAT_YUV
   0x01,                                        ///<  format index
   UVC_FRAME_NUM,                               ///<  Four frame descriptor for this format follows.
   UVC_GUID_FORMAT_YUY2,
   0x10,                                        ///<  bits per pixel
   0x01,                                        ///<  Default frame index is 1.
   0x00,                                        ///<  Non-interlaced stream 每 not required.
   0x00,                                        ///<  Non-interlaced stream 每 not required.
   0x00,                                        ///<  Non-interlaced stream
   0x00                                         ///<  No restrictions imposed on the duplication of this video stream.
};

const static T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR s_UvcUncompressedFrame[UVC_FRAME_NUM]=
{
	//frame1:1280x720
	{
        sizeof(T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR),        ///<Size of frame descriptor, in bytes.
        CS_INTERFACE,                                      ///<  CS_INTERFACE type
        VS_FRAME_UNCOMPRESSED,                             ///<mjpeg frame sub type
        0x01,                                              ///<frame index
        0x01,                                              ///<capabilities
        1280,                                               ///<width
        720,                                               ///<height
        1280*720*15*16,                                     ///<min bit rate
        1280*720*15*16,                                     ///<max bit rate
        1280*720*2,                                         ///<max frame buffer size
        0x000A2C2A,                                        ///<Default frame interval is 333333x100ns (30fps).
        0x00,                                              ///<  Continuous frame interval
        0x000A2C2A,                                        ///<  Minimum frame interval is 333333x100ns (30fps)
        0x000A2C2A,                                        ///<  Maximum frame interval is 333333x100ns(30fps).
        0x00000000,                                        ///<  No frame interval step supported.
    },
	//frame1:640x480
    {
        sizeof(T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR),        ///<Size of frame descriptor, in bytes.
        CS_INTERFACE,                                      ///<  CS_INTERFACE type
        VS_FRAME_UNCOMPRESSED,                             ///<mjpeg frame sub type
        0x02,                                              ///<frame index
        0x01,                                              ///<capabilities
        640,                                               ///<width
        480,                                               ///<height
        640*480*30*16,                                     ///<min bit rate
        640*480*30*16,                                     ///<max bit rate
        640*480*2,                                         ///<max frame buffer size
        0x00051615,                                        ///<Default frame interval is 333333x100ns (30fps).
        0x00,                                              ///<  Continuous frame interval
        0x00051615,                                        ///<  Minimum frame interval is 333333x100ns (30fps)
        0x00051615,                                        ///<  Maximum frame interval is 333333x100ns(30fps).
        0x00000000,                                        ///<  No frame interval step supported.
    },

    //frame2:320x240
    {
        sizeof(T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR),        ///<Size of frame descriptor, in bytes.
        CS_INTERFACE,                                      ///<  CS_INTERFACE type
        VS_FRAME_UNCOMPRESSED,                             ///<mjpeg frame sub type
        0x03,                                              ///<frame index
        0x01,                                              ///<capabilities
        320,                                               ///<width
        240,                                               ///<height
        320*240*30*16,                                     ///<min bit rate
        320*240*30*16,                                     ///<max bit rate
        320*240*2,                                         ///<max frame buffer size
        0x00051615,                                        ///<Default frame interval is 333333x100ns (30fps).
        0x00,                                              ///<  Continuous frame interval
        0x00051615,                                        ///<  Minimum frame interval is 333333x100ns (30fps)
        0x00051615,                                        ///<  Maximum frame interval is 333333x100ns(30fps).
        0x00000000,                                        ///<  No frame interval step supported.
    },
    //frame4:160x120
    {
        sizeof(T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR),        ///<Size of frame descriptor, in bytes.
        CS_INTERFACE,                                      ///<  CS_INTERFACE type
        VS_FRAME_UNCOMPRESSED,                             ///<mjpeg frame sub type
        0x04,                                              ///<frame index
        0x01,                                              ///<capabilities
        160,                                               ///<width
        120,                                               ///<height
        160*120*30*16,                                     ///<min bit rate
        160*120*30*16,                                     ///<max bit rate
        160*120*2,                                         ///<max frame buffer size
        0x00051615,                                        ///<Default frame interval is 333333x100ns (30fps).
        0x00,                                              ///<  Continuous frame interval
        0x00051615,                                        ///<  Minimum frame interval is 333333x100ns (30fps)
        0x00051615,                                        ///<  Maximum frame interval is 333333x100ns(30fps).
        0x00000000,                                        ///<  No frame interval step supported.
    },
};


static T_USB_ENDPOINT_DESCRIPTOR s_UvcEp2Desc =
{
    7,                                  ///<Descriptor size is 7 bytes 
    EP_DESC_TYPE,                       ///<ENDPOINT Descriptor Type
    (ENDPOINT_DIR_IN + EP2_INDEX),      ///<This is an IN endpoint with endpoint number 2  
    ENDPOINT_TYPE_BULK,                 ///<  Types - Transfer: BULK Pkt Size Adjust: No  
    EP2_BUF_MAX_LEN,                    ///<Maximum packet size for this endpoint is 512 Bytes. If Hi-Speed, 0 additional transactions per frame
    0x00                                ///< The polling interval value is every 0 Frames. If Hi-Speed, 0 uFrames/NAK  
};

/* language descriptor */
const static unsigned char s_UvcString0[] = 
{
    4,              ///<Descriptor size is 4 bytes  
    3,              ///< Second Byte of this descriptor     
    0x09, 0x04,     ///<Language Id: 1033  
};

/*string descriptor*/
const static unsigned char s_UvcString1[] =
{
    12,             
    0x03,               
    'A',0,          
    'N',0,
    'Y',0,
    'K',0,
    'A',0
};

const static unsigned char s_UvcString2[] = 
{
    12,             
    0x03,               
    'U',0,          
    'V',0,
    'C',0,
    '.',0,
    '.',0
};

const static unsigned char s_UvcString3[] = 
{
    12,             
    0x03,               
    '1',0,          
    '2',0,
    '3',0,
    '4',0,
    '5',0
};

T_VIDEO_PROBE_COMMIT_CONTROLS s_VideoProbeCommitControls =
{
    0x0000,             //  Bitmap
    0x01,               //  Video format index from a format descriptor.
    0x01,               //  Video frame index from a frame descriptor.
    0x000A2C2A,         //  Frame interval is 30fps  0x00051615    15P 
    0x0000,
    0x0000,
    0x0000,
    0x0000,
    0x0100,             //  Internal video streaming interface latency in ms from video data capture to presentation on the USB.
    0x0,                //  Max frame size
    128*1024,           //  Max payload size
};

static unsigned long s_UvcCtrlMap[UVC_SUPPORTED_CTRL_NUM]=
{
    UVC_CTRL_PARAM_GRP((UVC_PU_ID<<8 | UVC_VCI_ID),PU_BRIGHTNESS_CONTROL<<8),  //UVC_CTRL_BRIGHTNESS
    UVC_CTRL_PARAM_GRP((UVC_PU_ID<<8 | UVC_VCI_ID),PU_CONTRAST_CONTROL<<8),    //UVC_CTRL_CONTRAST 
    UVC_CTRL_PARAM_GRP((UVC_PU_ID<<8 | UVC_VCI_ID),PU_SATURATION_CONTROL<<8),  //UVC_CTRL_SATURATION    
    UVC_CTRL_PARAM_GRP((0<<8 | UVC_VSI_ID),VS_COMMIT_CONTROL<<8),               //UVC_CTRL_SATURATION 
    UVC_CTRL_PARAM_GRP((0<<8 | UVC_VSI_ID),VS_PROBE_CONTROL<<8),                //probe ctrl
    UVC_CTRL_PARAM_GRP((UVC_SU_ID<<8 | UVC_VCI_ID),SU_INPUT_SELECT_CONTROL<<8), //select unit ctrl
};
/**
* @brief get uvc frame resolution
* @author Huang Xin
* @date 2010-07-10
* @param pFrameRes[in] uvc frame resolution struct
* @param FrameId[in] uvc frame id
* @return void
*/
void uvc_get_frame_res(T_pUVC_FRAME_RES pFrameRes,unsigned long FrameId)
{
    if (NULL == pFrameRes || FrameId > UVC_FRAME_NUM || FrameId < 1)
    {
        akprintf(C3, M_DRVSYS, "uvc_get_frame_res():param error!\n");
        return;
    }
    pFrameRes->unHeight = s_UvcFramesRes[FrameId-1].unHeight;
    pFrameRes->unWidth = s_UvcFramesRes[FrameId-1].unWidth;
}

/** 
 * @brief set UVC callback
 *
 *This function is called by application level to set control callback.
 * @author Huang Xin
 * @date 2010-07-10
 * @param  vc_ctrl_callback[in] implement video control interface controls 
 * @param vs_ctrl_callback[in] implement video stream interface  controls 
 * @param frame_sent_callback[in] used to notify that a frame was sent completely
 * @return void
 */
void uvc_set_callback(T_pUVC_VC_CTRL_CALLBACK vc_ctrl_callback, 
                            T_pUVC_VS_CTRL_CALLBACK vs_ctrl_callback,  
                            T_pUVC_FRAME_SENT_CALLBACK frame_sent_callback)
{
    s_pUvcDev->fVcCtrlCallBack = vc_ctrl_callback;
    s_pUvcDev->fVsCtrlCallBack = vs_ctrl_callback;
    s_pUvcDev->fFrameSentCallBack = frame_sent_callback;
}

/**
* @brief	USB Video Class setup the advanced control features
*
* Implemented controls,such as brightness,contrast,saturation,called after uvc_init successful
* @author Huang Xin
* @date	2010-07-10
* @param dwControl[in] The advanced feature  control selector
* @param ulMin[in] The min value
* @param ulMax[in] The max value
* @param ulDef[in] The  def value
* @param ulResf[in] The res value
* @return bool
* @retval  false means failed
* @retval  AK_TURE means successful
*/
bool uvc_set_ctrl(T_eUVC_CONTROL dwControl, unsigned long ulMin, unsigned long ulMax, unsigned long ulDef,unsigned long ulRes)
{
    //  Check if the min and max are correct.
    if (ulMin >= ulMax)
    {
        return false;
    }  
    //  The default value should between min and max.
    if (ulDef < ulMin || ulDef > ulMax)
    {
        return false;
    }
    switch (dwControl)
    {
        //uvc device brightness control
        case UVC_CTRL_BRIGHTNESS:
            s_UvcProcessUnitDesc.bmControls |= 0x01;
            s_pUvcDev->tCtrlSetting[dwControl].ulLen = 0x2;
            s_pUvcDev->tCtrlSetting[dwControl].ucInfo = 0x3;
            break;
        //uvc device contrast control
        case UVC_CTRL_CONTRAST:   
            s_UvcProcessUnitDesc.bmControls |= 0x02;
            s_pUvcDev->tCtrlSetting[dwControl].ulLen = 0x2;
            s_pUvcDev->tCtrlSetting[dwControl].ucInfo = 0x3;
            break;
        //uvc device saturation control 
        case UVC_CTRL_SATURATION: 
            s_UvcProcessUnitDesc.bmControls |= 0x08;
            s_pUvcDev->tCtrlSetting[dwControl].ulLen = 0x2;
            s_pUvcDev->tCtrlSetting[dwControl].ucInfo = 0x3;
            break;
        default:
            akprintf(C3, M_DRVSYS, "unsupported ctrl!\n");
            return false;
    }
    s_pUvcDev->tCtrlSetting[dwControl].ulMin = ulMin;
    s_pUvcDev->tCtrlSetting[dwControl].ulMax = ulMax;
    s_pUvcDev->tCtrlSetting[dwControl].ulDef = ulDef;
    s_pUvcDev->tCtrlSetting[dwControl].ulCur = ulDef;
    s_pUvcDev->tCtrlSetting[dwControl].ulRes = ulRes;

    return true;
}

/**
* @brief Initialize uvc descriptor, MUST be called after uvc_set_ctrl
* @author Huang Xin
* @date	2010-07-10
* @return bool
* @retval  false means failed
* @retval  AK_TURE means successful

*/
bool uvc_init_desc()
{
    unsigned long cnt = 0;
    
    if (UVC_STREAM_MJPEG == s_pUvcDev->ucFormat)
    {
        s_UvcVideoStreamInterfaceInputHeaderDesc.wTotalLength = sizeof(T_UVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR) +
                                                             sizeof(T_UVC_MJPEG_VIDEO_FORMAT_DESCRIPTOR) +
                                                             sizeof(T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR) * UVC_FRAME_NUM;
        
        s_UvcConfigDesc.wTotalLength = s_UvcOtherSpeedConfigDesc.wTotalLength = sizeof(T_USB_CONFIGURATION_DESCRIPTOR) +
                                                                        sizeof(T_UVC_INTERFACE_ASSOCIATION_DESCRIPTOR) +
                                                                        sizeof(T_USB_INTERFACE_DESCRIPTOR) +
                                                                        s_UvcVideoCtrlInterfaceHeaderDesc.wTotalLength +
                                                                        sizeof(T_USB_ENDPOINT_DESCRIPTOR)+
                                                                        sizeof(T_UVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR) +
                                                                        sizeof(T_USB_INTERFACE_DESCRIPTOR) +
                                                                        s_UvcVideoStreamInterfaceInputHeaderDesc.wTotalLength +
                                                                        sizeof(T_USB_ENDPOINT_DESCRIPTOR);
    }
    else if (UVC_STREAM_YUV== s_pUvcDev->ucFormat)
    {
        s_UvcVideoStreamInterfaceInputHeaderDesc.wTotalLength = sizeof(T_UVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR) +
                                                             sizeof(T_UVC_UNCOMPRESSED_VIDEO_FORMAT_DESCRIPTOR) +
                                                             sizeof(T_UVC_VIDEOSTREAM_FRAME_DESCRIPTOR) * UVC_FRAME_NUM;
        
        s_UvcConfigDesc.wTotalLength = s_UvcOtherSpeedConfigDesc.wTotalLength = sizeof(T_USB_CONFIGURATION_DESCRIPTOR) +
                                                                        sizeof(T_UVC_INTERFACE_ASSOCIATION_DESCRIPTOR) +
                                                                        sizeof(T_USB_INTERFACE_DESCRIPTOR) +
                                                                        s_UvcVideoCtrlInterfaceHeaderDesc.wTotalLength +
                                                                        sizeof(T_USB_ENDPOINT_DESCRIPTOR)+
                                                                        sizeof(T_UVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR) +
                                                                        sizeof(T_USB_INTERFACE_DESCRIPTOR) +
                                                                        s_UvcVideoStreamInterfaceInputHeaderDesc.wTotalLength +
                                                                        sizeof(T_USB_ENDPOINT_DESCRIPTOR);
    }
    else
    {
        akprintf(C3, M_DRVSYS, "unsupported format!\n");
        return false;
    }
    memcpy(s_ConfigData,(unsigned char*)&s_UvcConfigDesc,sizeof(T_USB_CONFIGURATION_DESCRIPTOR));
    cnt += sizeof(T_USB_CONFIGURATION_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcInterfaceAssociationDesc,sizeof(T_UVC_INTERFACE_ASSOCIATION_DESCRIPTOR));
    cnt += sizeof(T_UVC_INTERFACE_ASSOCIATION_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcVciDesc,sizeof(T_USB_INTERFACE_DESCRIPTOR));
    cnt += sizeof(T_USB_INTERFACE_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcVideoCtrlInterfaceHeaderDesc,sizeof(T_UVC_VIDEOCONTROL_INTERFACE_HEADER_DESCRIPTOR));
    cnt += sizeof(T_UVC_VIDEOCONTROL_INTERFACE_HEADER_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcCameraTerminalDesc,sizeof(T_UVC_CAMERA_TERMINAL_DESCRIPTOR));
    cnt += sizeof(T_UVC_CAMERA_TERMINAL_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcInputTerminalDesc,sizeof(T_UVC_INPUT_TERMINAL_DESCRIPTOR));
    cnt += sizeof(T_UVC_INPUT_TERMINAL_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcOutputTerminalDesc,sizeof(T_UVC_OUTPUT_TERMINAL_DESCRIPTOR));
    cnt += sizeof(T_UVC_OUTPUT_TERMINAL_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcSelectUnitDesc,sizeof(T_UVC_SELECTOR_UNIT_DESCRIPTOR));
    cnt += sizeof(T_UVC_SELECTOR_UNIT_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcProcessUnitDesc,sizeof(T_UVC_PROCESSING_UNIT_DESCRIPTOR));
    cnt += sizeof(T_UVC_PROCESSING_UNIT_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcEp1Desc,sizeof(T_USB_ENDPOINT_DESCRIPTOR));
    cnt += sizeof(T_USB_ENDPOINT_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcVideoCtrlIntEpDesc,sizeof(T_UVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR) );
    cnt += sizeof(T_UVC_VIDEOCONTROL_INTERRUPT_ENDPOINT_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcVsiDesc,sizeof(T_USB_INTERFACE_DESCRIPTOR));
    cnt += sizeof(T_USB_INTERFACE_DESCRIPTOR);
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcVideoStreamInterfaceInputHeaderDesc, sizeof(T_UVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR));
    cnt +=  sizeof(T_UVC_VIDEOSTREAM_INTERFACE_INPUT_HEADER_DESCRIPTOR);
    if (UVC_STREAM_MJPEG == s_pUvcDev->ucFormat)
    {
        memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcMjpegFormatDesc, sizeof(T_UVC_MJPEG_VIDEO_FORMAT_DESCRIPTOR));
        cnt +=  sizeof(T_UVC_MJPEG_VIDEO_FORMAT_DESCRIPTOR);
        memcpy(s_ConfigData+cnt,(unsigned char*)s_UvcMjpegFrame, sizeof(s_UvcMjpegFrame));
        cnt +=  sizeof(s_UvcMjpegFrame);
    }
    else
    {
        memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcUncompressedFormatDesc, sizeof(T_UVC_UNCOMPRESSED_VIDEO_FORMAT_DESCRIPTOR));
        cnt += sizeof(T_UVC_UNCOMPRESSED_VIDEO_FORMAT_DESCRIPTOR);
        memcpy(s_ConfigData+cnt,(unsigned char*)s_UvcUncompressedFrame, sizeof(s_UvcUncompressedFrame));
        cnt +=  sizeof(s_UvcUncompressedFrame);
    }
    memcpy(s_ConfigData+cnt,(unsigned char*)&s_UvcEp2Desc,sizeof(T_USB_ENDPOINT_DESCRIPTOR));
    cnt += sizeof(T_USB_ENDPOINT_DESCRIPTOR);
    if (cnt != s_UvcConfigDesc.wTotalLength)
    {
        akprintf(C1, M_DRVSYS, "all config desc size error %d\n",cnt);
        return false;
    }
    akprintf(C1, M_DRVSYS, "init all config desc ok: %d\n",cnt);
    return true;
}

/**
* @brief Initialize uvc descriptor,yuv buffer,and map msg
* @author Huang Xin
* @date	2010-07-10
* @param mode[in] usb2.0 or usb1.0 
* @param format[in] The UVC frame format
* @return bool
* @retval  false means failed
* @retval  AK_TURE means successful
*/
bool uvc_init(unsigned long mode,unsigned char format)
{
    unsigned char i;
   
    s_pUvcDev = (T_pUVC_DEV)drv_malloc(sizeof(T_UVC_DEV));
    if (s_pUvcDev == NULL)
    {
        akprintf(C1, M_DRVSYS, "s_pUvcDev,alloc failed\n");
        return false;
    }  
    //init s_pUvcDev member
    memset(s_pUvcDev,0,sizeof(T_UVC_DEV));
    s_pUvcDev->ulMode = mode;
    s_pUvcDev->ucFormat = format;
    for (i = UVC_CTRL_NUM; i<UVC_SUPPORTED_CTRL_NUM; i++)
    {
        s_pUvcDev->tCtrlSetting[i].ucInfo = 1;
    }
    //map message
    DrvModule_Map_Message(DRV_MODULE_UVC, UVC_DEV_MSG_CTRL, uvc_dev_msg_ctrl_proc);
    DrvModule_Map_Message(DRV_MODULE_UVC, UVC_DEV_MSG_SEND, uvc_dev_msg_send_proc);

    return true;
}

/**
* @brief	USB Video Class payload packing function
* @author Huang Xin
* @date	2010-07-10
* @param pPayload[out] The buffer to store the payload
* @param pData[in] The original frame data to be packed
* @param dwSize[in] The size of the frame
* @return unsigned long
* @retval The size of the payload
*/
unsigned long uvc_payload(unsigned char* pPayload, unsigned char* pData, unsigned long dwSize)
{
    static unsigned char fid = 0;
    unsigned long i, j;
    unsigned char* pTmp;
    
    if (pPayload == NULL || pData == NULL)
    {
        akprintf(C1, M_DRVSYS, "error:payload buffer or data buffer null\n");
        return 0;
    }
    i = dwSize;
    j = 0;
    pTmp = pData;
    fid = !fid;
    while (i > 0)
    {
        pPayload[j] = 0x02;
        pPayload[j + 1] = 0x80|fid;

        if (i > s_VideoProbeCommitControls.dwMaxPayloadTransferSize - 2)
        {
            memcpy(&pPayload[j + 2], pTmp, s_VideoProbeCommitControls.dwMaxPayloadTransferSize - 2);
            pTmp += s_VideoProbeCommitControls.dwMaxPayloadTransferSize - 2;
            j += s_VideoProbeCommitControls.dwMaxPayloadTransferSize;
            i -= s_VideoProbeCommitControls.dwMaxPayloadTransferSize - 2;
        }
        else
        {
            memcpy(&pPayload[j + 2], pTmp, i);
            pPayload[j + 1] |= 2;
            j += i + 2;
            i = 0;
        }
    }

    return j;
}

/**
* @brief	Send frame data via usb
* @author Huang Xin
* @date	2010-07-10
* @param data_buf[in] : buffer to be send. 
* @param length[in]: length of the buffer
* @return bool
* @retval  false means failed
* @retval  AK_TURE means successful

*/
bool uvc_send(unsigned char *data_buf, unsigned long length)
{
    unsigned long ret = 0;
    
    if (NULL == data_buf)
    {
        return false;
    }
    if (!g_bUvcSending)
    {

        //printf("g_bUvcsening = false\n");
        return false;
    }
    ret = usb_slave_data_in(EP2_INDEX, data_buf, length);
    //printf("ret = 0 \n");
    if(ret == 0)
        return false;

    return true;
    
}



/**
* @brief	Start UVC
* @author Huang Xin
* @date	2010-07-10
* @return bool
*/
bool uvc_start()
{
    if (FREQ_INVALID_HANDLE == freq_handle)
    {
        freq_handle = FreqMgr_RequestFreq(FREQ_REQUST_ASIC);
    }

    //create task
    if (!DrvModule_Create_Task(DRV_MODULE_UVC))
        return false;
    if(!uvc_enable())
        return false;
    return true;
}

/**
* @brief stop UVC
* @author Huang Xin
* @date	2010-07-10
* @return VOID
*/
void uvc_stop(void)
{
    g_bUvcSending = false;
    
    DrvModule_Terminate_Task(DRV_MODULE_UVC);
    uvc_disable();
    if (NULL != s_pUvcDev)
    {
        drv_free(s_pUvcDev);
    }

    FreqMgr_CancelFreq(freq_handle);
    freq_handle = FREQ_INVALID_HANDLE;
    
}

/**
* @brief check if UVC opened
* @author Huang Xin
* @date	2010-07-10
* @return bool
* @retval true: opened, false: not opened
*/
bool uvc_check_open(void)
{
    return g_bUvcSending;
}
/**
* @brief USB Video Class  parse yuv function
* @author Huang Xin
* @date	2010-07-10
* @param pYUV[out] The buffer to store the YUV frame
* @param y[in] The original y param addr
* @param u[in] The original u param addr
* @param v[in] The original v param addr
* @param width[in] The width of the yuv frame
* @param height[in] The height of the yuv frame
* @param yuv_format[in] The format of  yuv,yuv422 or yuv420
* @return unsigned long
* @retval The size of the yuv frame
*/
unsigned long uvc_parse_yuv(unsigned char *pYUV,unsigned char *y,unsigned char *u,unsigned char *v,unsigned long width,unsigned long height,unsigned char yuv_format)
{
    unsigned char* dy,*du,*dv;
    unsigned long i = 0;
    unsigned long j = 0;
    unsigned long tmp = 0;

    if (pYUV == NULL || y == NULL || u == NULL || v == NULL)
    {
        return 0;
    }
    //yuv 4:2:2
    if(yuv_format == YUV_FORMAT_422)
    {
        dy=y;
        du=u;
        dv=v;
        tmp = width * height * 2;
        for(i=0; i<tmp; i+=4)
        {
            pYUV[i] = *dy++;
            pYUV[i+1] = *du++;
            pYUV[i+2] = *dy++;
            pYUV[i+3] = *dv++;
        }
    }
    //yuv420 to yuv422
    else if (yuv_format == YUV_FORMAT_420)
    {
              
        for (i = 0; i < height; i++) 
        { 
            dy = y + i * width; 
            du = u + (i / 2) * (width / 2); 
            dv = v + (i / 2) * (width / 2); 
            for (j = 0; j < width; j += 2)
            { 
                *(pYUV + 0) = dy[0]; 
                *(pYUV + 1) = du[0]; 
                *(pYUV + 2) = dy[1]; 
                *(pYUV + 3) = dv[0]; 
                pYUV += 4; 
                dy += 2; 
                ++du; 
                ++dv; 
            }
        }
    }
    else
    { 
        akprintf(C3, M_DRVSYS, "yuv format error\n");
        return 0;
    }
    return width*height*2;

    
}
/**
 * @brief   uvc dev msg process 
 *
 * called by uvc task when uvc dev msg sent successful
 * @author Huang Xin
 * @date 2010-08-04
 * @param pMsg[in] uvc msg parameter
 * @param len[in] uvc msg parameter len
 * @return  void
 */
static void uvc_dev_msg_ctrl_proc(unsigned long* pMsg,unsigned long len)
{
    unsigned char ctrl_id;
    T_pUVC_DEV_MSG message = (T_pUVC_DEV_MSG)pMsg;

    switch (message->ucMsgId)
    {
        case MSG_VC_CTRL: 
            for (ctrl_id = 0; ctrl_id < UVC_CTRL_NUM; ctrl_id++)
            {
                if (s_UvcCtrlMap[ctrl_id] == UVC_CTRL_PARAM_GRP(message->ucParam1<<8 | UVC_VCI_ID, message->ucParam2<<8))
                {
                    break;
                }
            }
            //vc ctrl porc inside drvlib
            if (ctrl_id >= UVC_CTRL_NUM)
            {
                vc_ctrl_handle(message->ucParam1,message->ucParam2,message->ulParam3,message->ulParam4);
            }
            //vc ctrl porc outside drvlib
            else
            {
                s_pUvcDev->fVcCtrlCallBack(ctrl_id,message->ulParam3,message->ulParam4);
            }
            break;
        case MSG_VS_CTRL:
            for (ctrl_id = 0; ctrl_id < UVC_CTRL_NUM; ctrl_id++)
            {
                if (s_UvcCtrlMap[ctrl_id] == UVC_CTRL_PARAM_GRP(message->ucParam1<<8 | UVC_VSI_ID, message->ucParam2<<8))
                {
                    break;
                }
            }
            //vs ctrl porc inside drvlib
            if (ctrl_id >= UVC_CTRL_NUM)
            {
                vs_ctrl_handle(message->ucParam1,message->ucParam2,message->ulParam3,message->ulParam4);
            }
            //vs ctrl porc outside drvlib
            else
            {
                if (NULL != s_pUvcDev->fVsCtrlCallBack)
                {
                    g_bUvcSending = true;
                    s_pUvcDev->fVsCtrlCallBack(ctrl_id,message->ulParam3,message->ulParam4); 
                }
            }
         
            break;
        default :
            break;
    }
}   

static void uvc_dev_msg_send_proc(unsigned long* pMsg,unsigned long len)
{
        if (NULL != s_pUvcDev->fFrameSentCallBack)
        {
            s_pUvcDev->fFrameSentCallBack(); 
        }

}

/**
 * @brief   uvc vc ctrl handle 
 *
 * called by uvc task when uvc vc ctrl is processed inside drvlib
 * @author Huang Xin
 * @date 2010-08-04
 * @param unit[in] uvc ctrl unit id
 * @param cs[in] uvc ctrl selector
 * @param cur_val1[in] uvc vc ctrl data
 * @param cur_val2[in] uvc vc ctrl data
 * @return  void
 */
static void vc_ctrl_handle(unsigned char unit, unsigned char cs , unsigned long cur_val1,unsigned long cur_val2)
{
    akprintf(C3, M_DRVSYS, "vc ctrl handle\n");
}

/**
 * @brief   uvc vs ctrl handle 
 *
 * called by uvc task when uvc vs ctrl is processed inside drvlib
 * @author Huang Xin
 * @date 2010-08-04
 * @param unit[in] uvc vs ctrl unit id
 * @param cs[in] uvc vs ctrl selector
 * @param cur_val1[in] uvc vs ctrl data
 * @param cur_val2[in] uvc vs ctrl data
 * @return  void
 */
static void vs_ctrl_handle(unsigned char unit, unsigned char cs , unsigned long cur_val1,unsigned long cur_val2)
{   
    switch (cs)
    {
        case VS_PROBE_CONTROL:
            break;
        case VS_COMMIT_CONTROL:
            break;
        default:
            akprintf(C3, M_DRVSYS, "unsupported vs ctrl\n");
            break;              
    }
    
}


/**
 * @brief   uvc enable 
 *
 * called by uvc_start() when start uvc task successful
 * @author Huang Xin
 * @date 2010-08-04
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
static bool uvc_enable()
{
    Usb_Slave_Standard.usb_get_device_descriptor = uvc_get_dev_desc;
    Usb_Slave_Standard.usb_get_config_descriptor = uvc_get_cfg_desc;
    Usb_Slave_Standard.usb_get_string_descriptor = uvc_get_str_desc;
    Usb_Slave_Standard.usb_get_device_qualifier_descriptor = uvc_get_dev_qualifier_desc;
    Usb_Slave_Standard.usb_get_other_speed_config_descriptor = uvc_get_other_speed_cfg_desc;
    Usb_Slave_Standard.Device_ConfigVal =           0;
    Usb_Slave_Standard.Device_Address =             0;
    Usb_Slave_Standard.Buffer  =             (unsigned char *)drv_malloc(4096); 
    Usb_Slave_Standard.buf_len =            4096;
    
    //usb slave init,alloc L2 buffer,register irq
    usb_slave_init(Usb_Slave_Standard.Buffer, Usb_Slave_Standard.buf_len);
    //usb std init,set ctrl callback
    usb_slave_std_init();
    //set class req callback
    usb_slave_set_ctrl_callback(REQUEST_CLASS, uvc_class_callback);    
    usb_slave_set_callback(uvc_reset,uvc_suspend,uvc_resume, uvc_configok);
    usb_slave_set_tx_callback(EP2_INDEX, uvc_send_finish);
    //usb_slave_set_rx_callback(EP3_INDEX, uvc_receive_notify, uvc_receive_finish);
    usb_slave_device_enable(s_pUvcDev->ulMode);
    akprintf(C3, M_DRVSYS, "uvc enable ok\n");
    return true;
}
/**
 * @brief   uvc disable 
 *
 * called by uvc_stop() when terminate uvc task successful
 * @author Huang Xin
 * @date 2010-08-04
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
static bool uvc_disable()
{
    drv_free(Usb_Slave_Standard.Buffer);
    memset(&Usb_Slave_Standard,0,sizeof(Usb_Slave_Standard));
    usb_slave_device_disable();
    
    //clear  callback
    usb_slave_set_ctrl_callback(REQUEST_CLASS, NULL);   
    usb_slave_set_callback(NULL,NULL,NULL, NULL);
    usb_slave_set_tx_callback(EP2_INDEX, NULL);
    
    usb_slave_free();
    return true;
}


/**
 * @brief  reset callback
 *
 * called  when usb reset
 * @author Huang Xin
 * @date 2010-08-04
 * @param mode[in] usb1.1 or usb2.0
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
static void uvc_reset(unsigned long mode)
{
    unsigned short cnt = 0;

    //clr stall status to avoid die when uvc task is waiting clr stall to send csw 
    usb_slave_set_ep_status(EP2_INDEX,0);
    cnt = s_UvcConfigDesc.wTotalLength;
    if(mode == USB_MODE_20)
    {
        s_ConfigData[cnt-3]=0x00;
        s_ConfigData[cnt-2]=0x02; 
    }
    else
    {
        s_ConfigData[cnt-3]=0x40;
        s_ConfigData[cnt-2]=0x00;  
    }
    //reinit s_pUvcDev member,usb1.1 or usb2.0
    s_pUvcDev->ulMode = mode;  


}

/**
 * @brief  suspend callback
 *
 * called  when usb suspend
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void uvc_suspend()
{
    akprintf(C3, M_DRVSYS, "uvc suspend \n");
    //clr stall status to avoid die when uvc task is waiting clr stall to send csw 
    usb_slave_set_ep_status(EP2_INDEX,0);
    usb_slave_set_ep_status(EP3_INDEX,0);
}

/**
 * @brief  resume callback
 *
 * called  when usb resume
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void uvc_resume()
{
    akprintf(C3, M_DRVSYS, "uvc resume \n");
}

/**
 * @brief config ok callback
 *
 * called  when enum successful
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void uvc_configok()
{    
    akprintf(C3, M_DRVSYS, "ok!\n");

}
/**
 * @brief  bulk in ep send finish callback
 *
 * called  when bulk in ep send finish
 * @author Huang Xin
 * @date 2010-08-04
 * @return  void
 */
static void uvc_send_finish()
{
    DrvModule_Send_Message(DRV_MODULE_UVC, UVC_DEV_MSG_SEND, NULL);
}

/**
 * @brief   set uvc class req callback
 *
 * called by usb drv  when msc class req is received successful
 * @author Huang Xin
 * @date 2010-08-04
 * @param pTrans[in] ctrl trans struct
 * @return  bool
 * @retval  false means failed
 * @retval  AK_TURE means successful
 */
static bool uvc_class_callback(T_CONTROL_TRANS *pTrans)
{
    bool ret = true;
    unsigned char req_type;
    unsigned char ctrl_id;
    T_UVC_DEV_MSG msg;


    if (RECIPIENT_INTERFACE == (pTrans->dev_req.bmRequestType & 0x1f))
    {
        switch (pTrans->stage)
        {
            case CTRL_STAGE_SETUP: 
                for (ctrl_id = 0; ctrl_id < UVC_SUPPORTED_CTRL_NUM; ctrl_id++)
                {
                    if (s_UvcCtrlMap[ctrl_id] == UVC_CTRL_PARAM_GRP(pTrans->dev_req.wIndex, pTrans->dev_req.wValue))
                    {
                        break;
                    }
                }
                if (ctrl_id >= UVC_SUPPORTED_CTRL_NUM)
                {
                    akprintf(C3, M_DRVSYS, "GET: vc ctrl unsupported,stall ep0\n");
                    ret = false;
                }
                else
                {   
                    switch (pTrans->dev_req.bRequest)
                    {
                        case GET_CUR:
                            akprintf(C3, M_DRVSYS, "GET_CUR\n");     
                            switch (LOBYTE(pTrans->dev_req.wIndex))
                            {
                                case UVC_VCI_ID :
                                    switch (HIBYTE(pTrans->dev_req.wIndex))
                                    {
                                        case UVC_PU_ID :       
                                            memcpy(pTrans->buffer,(unsigned char *)(&s_pUvcDev->tCtrlSetting[ctrl_id].ulCur),s_pUvcDev->tCtrlSetting[ctrl_id].ulLen);
                                            pTrans->data_len = s_pUvcDev->tCtrlSetting[ctrl_id].ulLen;
                                            break;         
                                        default:
                                            akprintf(C1, M_DRVSYS, "Unsupport GET_CUR:Unit\n");
                                            //ep0 will stall
                                            ret = false;
                                            break;
                                    }
                                    break;
                                case UVC_VSI_ID :
                                    switch (HIBYTE(pTrans->dev_req.wValue))
                                    {
                                        case VS_PROBE_CONTROL:
                                            memcpy(pTrans->buffer,(unsigned char *)(&s_VideoProbeCommitControls),sizeof(T_VIDEO_PROBE_COMMIT_CONTROLS));
                                            pTrans->data_len = sizeof(T_VIDEO_PROBE_COMMIT_CONTROLS);
                                            break;

                                        default:
                                            akprintf(C1, M_DRVSYS, "GET_CUR:invalid ctrl selector\n");
                                            //ep0 will stall
                                            ret = false;
                                            break;
                                    }
                                    break;

                                default:
                                    ret = false;
                                    break;
                            }
                            break;
                        case GET_MIN:
                            akprintf(C3, M_DRVSYS, "GET_MIN\n");     
                            switch (LOBYTE(pTrans->dev_req.wIndex))
                            {
                                case UVC_VCI_ID :
                                    switch (HIBYTE(pTrans->dev_req.wIndex))
                                    {
                                        case UVC_PU_ID :       
                                            memcpy(pTrans->buffer,(unsigned char *)(&s_pUvcDev->tCtrlSetting[ctrl_id].ulMin),s_pUvcDev->tCtrlSetting[ctrl_id].ulLen);
                                            pTrans->data_len = s_pUvcDev->tCtrlSetting[ctrl_id].ulLen;
                                            break;
                                        
                                        default:
                                            akprintf(C1, M_DRVSYS, "Unsupport GET_MIN:Unit\n");
                                            //ep0 will stall
                                            ret = false;
                                            break;
                                    }
                                    break;
                                case UVC_VSI_ID :
                                    switch (HIBYTE(pTrans->dev_req.wValue))
                                    {
                                        case VS_PROBE_CONTROL:
                                            memcpy(pTrans->buffer,(unsigned char *)(&s_VideoProbeCommitControls),sizeof(T_VIDEO_PROBE_COMMIT_CONTROLS));
                                            pTrans->data_len = sizeof(T_VIDEO_PROBE_COMMIT_CONTROLS);
                                            break;

                                        default:
                                            akprintf(C1, M_DRVSYS, "GET_MIN:invalid ctrl selector\n");
                                            //ep0 will stall
                                            ret = false;
                                            break;
                                    }
                                    break;

                                default:
                                    ret = false;
                                    break;
                            }
                            break;
                        case GET_MAX:
                            akprintf(C3, M_DRVSYS, "GET_MAX\n");     
                            switch (LOBYTE(pTrans->dev_req.wIndex))
                            {
                                case UVC_VCI_ID :
                                    switch (HIBYTE(pTrans->dev_req.wIndex))
                                    {
                                        case UVC_PU_ID :       
                                            memcpy(pTrans->buffer,(unsigned char *)(&s_pUvcDev->tCtrlSetting[ctrl_id].ulMax),s_pUvcDev->tCtrlSetting[ctrl_id].ulLen);
                                            pTrans->data_len = s_pUvcDev->tCtrlSetting[ctrl_id].ulLen;
                                            break;
                                        
                                        default:
                                            akprintf(C1, M_DRVSYS, "Unsupport GET_MAX:Unit\n");
                                            //ep0 will stall
                                            ret = false;
                                            break;
                                    }
                                    break;
                                case UVC_VSI_ID:
                                    switch (HIBYTE(pTrans->dev_req.wValue))
                                    {
                                        case VS_PROBE_CONTROL:
                                            memcpy(pTrans->buffer,(unsigned char *)(&s_VideoProbeCommitControls),sizeof(T_VIDEO_PROBE_COMMIT_CONTROLS));
                                            pTrans->data_len = sizeof(T_VIDEO_PROBE_COMMIT_CONTROLS);
                                            break;

                                        default:
                                            akprintf(C1, M_DRVSYS, "GET_MAX:invalid ctrl selector\n");
                                            //ep0 will stall
                                            ret = false;
                                            break;
                                    }
                                    break;

                                default:
                                    ret = false;
                                    break;
                            }
                            break;
                        case GET_RES:
                            akprintf(C3, M_DRVSYS, "GET_RES\n");     
                            switch (LOBYTE(pTrans->dev_req.wIndex))
                            {
                                case UVC_VCI_ID :
                                    switch (HIBYTE(pTrans->dev_req.wIndex))
                                    {
                                        case UVC_PU_ID :       
                                            memcpy(pTrans->buffer,(unsigned char *)(&s_pUvcDev->tCtrlSetting[ctrl_id].ulRes),s_pUvcDev->tCtrlSetting[ctrl_id].ulLen);
                                            pTrans->data_len = s_pUvcDev->tCtrlSetting[ctrl_id].ulLen;
                                            break;
                                        
                                        default:
                                            akprintf(C1, M_DRVSYS, "Unsupport GET_RES:Unit\n");
                                            //ep0 will stall
                                            ret = false;
                                            break;
                                    }
                                    break;
                                case UVC_VSI_ID:
                                    akprintf(C1, M_DRVSYS, "Unsupport GET_RES: VSI\n");
                                    ret = false;
                                    break;

                                default:
                                    ret = false;
                                    break;
                            }
                            break;
                        case GET_LEN:
                            akprintf(C1, M_DRVSYS, "Unsupport GET_LEN\n");
                            ret = false;
                            break;
                        case GET_INFO:
                            akprintf(C3, M_DRVSYS, "GET_INFO\n");     
                            switch (LOBYTE(pTrans->dev_req.wIndex))
                            {
                                case UVC_VCI_ID :
                                    switch (HIBYTE(pTrans->dev_req.wIndex))
                                    {
                                        case UVC_PU_ID :       
                                            pTrans->buffer[0] = s_pUvcDev->tCtrlSetting[ctrl_id].ucInfo;
                                            pTrans->data_len = 1;
                                            break;
                                        case UVC_SU_ID :
                                            pTrans->buffer[0] = s_pUvcDev->tCtrlSetting[ctrl_id].ucInfo;
                                            pTrans->data_len = 1;
                                            break;
                                        default:
                                            akprintf(C1, M_DRVSYS, "Unsupport GET_INFO:Unit\n");
                                            //ep0 will stall
                                            ret = false;
                                            break;
                                    }
                                    break;
                                case UVC_VSI_ID:
                                    akprintf(C1, M_DRVSYS, "Unsupport GET_RES: VSI\n");
                                    ret = false;
                                    break;

                                default:
                                    ret = false;
                                    break;
                            }
                            break;
                        case GET_DEF:
                            akprintf(C3, M_DRVSYS, "GET_DEF\n");     
                            switch (LOBYTE(pTrans->dev_req.wIndex))
                            {
                                case UVC_VCI_ID :
                                    switch (HIBYTE(pTrans->dev_req.wIndex))
                                    {
                                        case UVC_PU_ID :       
                                            memcpy(pTrans->buffer,(unsigned char *)(&s_pUvcDev->tCtrlSetting[ctrl_id].ulDef),s_pUvcDev->tCtrlSetting[ctrl_id].ulLen);
                                            pTrans->data_len = s_pUvcDev->tCtrlSetting[ctrl_id].ulLen;
                                            break;
                                        
                                        default:
                                            akprintf(C1, M_DRVSYS, "Unsupport GET_DEF:Unit\n");
                                            //ep0 will stall
                                            ret = false;
                                            break;
                                    }
                                    break;
                                case UVC_VSI_ID :
                                    switch (HIBYTE(pTrans->dev_req.wValue))
                                    {
                                        case VS_PROBE_CONTROL:
                                            memcpy(pTrans->buffer,(unsigned char *)(&s_VideoProbeCommitControls),sizeof(T_VIDEO_PROBE_COMMIT_CONTROLS));
                                            pTrans->data_len = sizeof(T_VIDEO_PROBE_COMMIT_CONTROLS);
                                            break;

                                        default:
                                            akprintf(C1, M_DRVSYS, "GET_DEF:invalid ctrl selector:%x\n", HIBYTE(pTrans->dev_req.wValue));
                                            //ep0 will stall
                                            ret = false;
                                            break;
                                    }
                                    break;

                                default:
                                    ret = false;
                                    break;
                            }
                            break;
                        //NOT GET REQ
                        default:
                            break;
                    }
                }
                break;
            case CTRL_STAGE_DATA_OUT:
                if (pTrans->dev_req.bRequest == SET_CUR)
                {
                    akprintf(C3, M_DRVSYS, "SET_CUR\n");
                    switch (LOBYTE(pTrans->dev_req.wIndex))
                    {
                        //  Video Control Interface
                        case UVC_VCI_ID :
                            msg.ucMsgId = MSG_VC_CTRL; 
                            msg.ucParam1 = HIBYTE(pTrans->dev_req.wIndex);
                            msg.ucParam2 = HIBYTE(pTrans->dev_req.wValue);
                            msg.ulParam3 = *((unsigned long *)(pTrans->buffer));
                            msg.ulParam4 = *((unsigned long *)(pTrans->buffer+4));
                            DrvModule_Send_Message(DRV_MODULE_UVC, UVC_DEV_MSG_CTRL, (unsigned long*)&msg);
                            break;
                        // video stream interface
                        case UVC_VSI_ID :
                            msg.ucMsgId = MSG_VS_CTRL;
                            switch (HIBYTE(pTrans->dev_req.wValue))
                            {
                                case VS_PROBE_CONTROL:

                                    memcpy((unsigned char *)&s_VideoProbeCommitControls,(unsigned char *)pTrans->buffer,4); 
                                    if (s_pUvcDev->ucFormat == UVC_STREAM_MJPEG)
                                    {
                                        s_VideoProbeCommitControls.dwMaxVideoFrameSize = s_UvcMjpegFrame[s_VideoProbeCommitControls.bFrameIndex- 1].dwMaxVideoFrameBufSize;
                                    }
                                    else
                                    {
                                        s_VideoProbeCommitControls.dwMaxVideoFrameSize = s_UvcUncompressedFrame[s_VideoProbeCommitControls.bFrameIndex - 1].dwMaxVideoFrameBufSize;
                                    } 
                                    akprintf(C3, M_DRVSYS, "vs prob, frmID:%d \n", s_VideoProbeCommitControls.bFrameIndex);                                  

                                    break;
                                case VS_COMMIT_CONTROL :
                                    msg.ucParam1 = HIBYTE(pTrans->dev_req.wIndex);
                                    msg.ucParam2 = HIBYTE(pTrans->dev_req.wValue);
                                    msg.ulParam3 = *((unsigned long *)(pTrans->buffer));
                                    msg.ulParam4 = *((unsigned long *)(pTrans->buffer+4));
                                    DrvModule_Send_Message(DRV_MODULE_UVC, UVC_DEV_MSG_CTRL, (unsigned long*)&msg);
                                    break;
                                default:
                                    akprintf(C1, M_DRVSYS, "GET_CUR:invalid ctrl selector\n");
                                    //ep0 will stall
                                    ret = false;
                                    break;
                            }
                            break;
                        default:
                            akprintf(C1, M_DRVSYS, "invalid dev_req.wIndex\n");
                            return false;
                    }

                }
                break;
            case CTRL_STAGE_DATA_IN:
            case CTRL_STAGE_STATUS:             
                break;
            default:
                break;
        }
    }
    else
    {
        akprintf(C3, M_DRVSYS, " not RECIPIENT_INTERFACE ,stall ep0\n");
        ret = false;
    }
    return ret;
    
}




/** 
 * @brief get dev qualifier descriptor callback
 *
 * this callback is set at uvc_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of dev qualifier descriptor
 * @return  unsigned char *
 * @retval  addr of dev qualifier descriptor
 */
static unsigned char *uvc_get_dev_qualifier_desc(unsigned long *count)
{
    *count = sizeof(s_UvcDeviceQualifierDesc);
    return (unsigned char *)&s_UvcDeviceQualifierDesc;
}

/** 
 * @brief get dev descriptor callback
 *
 * this callback is set at uvc_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of dev descriptor
 * @return  unsigned char *
 * @retval  addr of dev descriptor
 */
static unsigned char *uvc_get_dev_desc(unsigned long *count)
{
    *count = sizeof(s_UvcDeviceDesc);
    return (unsigned char *)&s_UvcDeviceDesc;
}

/** 
 * @brief get all config descriptor callback
 *
 * this callback is set at uvc_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of all config descriptor
 * @return  unsigned char *
 * @retval  addr of all config descriptor
 */
static unsigned char *uvc_get_cfg_desc(unsigned long *count)
{ 
    *count = s_UvcConfigDesc.wTotalLength;  
    return s_ConfigData;
}

/** 
 * @brief get other speed config descriptor callback
 *
 * this callback is set at uvc_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param count[out] len of other speed config descriptor
 * @return  unsigned char *
 * @retval  addr of other speed config descriptor
 */
static unsigned char * uvc_get_other_speed_cfg_desc(unsigned long *count)
{
    unsigned short cnt = 0;

    memcpy(s_ConfigData, (unsigned char *)&s_UvcOtherSpeedConfigDesc, *(unsigned char *)&s_UvcOtherSpeedConfigDesc);
    cnt = s_UvcOtherSpeedConfigDesc.wTotalLength;
    //other speed is full speed,ep2 maxpktsize is 64  
    if (s_pUvcDev->ulMode == USB_MODE_20)
    {
        s_ConfigData[cnt-3]=0x40;
        s_ConfigData[cnt-2]=0x00;   
    }
    //other speed is high speed,ep2 maxpktsize is 512  
    else
    {
        s_ConfigData[cnt-3]=0x00;
        s_ConfigData[cnt-2]=0x02;
    }
    *count = cnt;
    return s_ConfigData;
}

/** 
 * @brief get string descriptor callback
 *
 * this callback is set at uvc_enable()
 * @author Huang Xin
 * @date 2010-08-04
 * @param index[in] index of string descriptor
 * @param count[out] len of stirng descriptor
 * @return  unsigned char *
 * @retval  addr of string descriptor
 */
static unsigned char *uvc_get_str_desc(unsigned char index, unsigned long *count)
{
    if(index == 0)
    {
        *count = sizeof(s_UvcString0);
        return (unsigned char *)s_UvcString0;
    }
    else if(index == 1)
    {
        *count = sizeof(s_UvcString1);
        return (unsigned char *)s_UvcString1;
    }
    else if(index == 2)
    {
        *count = sizeof(s_UvcString2);
        return (unsigned char *)s_UvcString2;
    }
      else if(index == 3)
    {
        *count = sizeof(s_UvcString3);
        return (unsigned char *)s_UvcString3;
    }
    return NULL;
}

