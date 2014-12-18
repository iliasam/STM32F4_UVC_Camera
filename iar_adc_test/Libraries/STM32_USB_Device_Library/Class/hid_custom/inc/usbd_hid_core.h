/**
  ******************************************************************************
  * @file    usbd_hid_core.h
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   header file for the usbd_hid_core.c file.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/

#ifndef __USB_HID_CORE_H_
#define __USB_HID_CORE_H_

#include  "usbd_ioreq.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_HID
  * @brief This file is the Header file for usbd_hid_core.c
  * @{
  */ 


/** @defgroup USBD_HID_Exported_Defines
  * @{
  */ 
#define USB_HID_CONFIG_DESC_SIZ        41
#define USB_HID_DESC_SIZ                9	//CUSTOMHID_SIZ_HID_DESC
#define CUSTOMHID_SIZ_REPORT_DESC	  135
#define HID_MOUSE_REPORT_DESC_SIZE     74


#define HID_DESCRIPTOR_TYPE           0x21
#define HID_REPORT_DESC               0x22


#define HID_REQ_SET_PROTOCOL          0x0B
#define HID_REQ_GET_PROTOCOL          0x03

#define HID_REQ_SET_IDLE              0x0A
#define HID_REQ_GET_IDLE              0x02

#define HID_REQ_SET_REPORT            0x09
#define HID_REQ_GET_REPORT            0x01

/* ********************************************************** */
/*  HID report types                                          */
/* ********************************************************** */
#define hrINPUT						0x81
#define hrOUTPUT					0x91
#define hrFEATURE					0xb1
#define 	hrDATA					0x00
#define 	hrCONSTANT				0x01
#define 	hrARRAY					0x00
#define 	hrVARIABLE				0x02
#define 	hrABSOLUTE				0x00
#define 	hrRELATIVE				0x04
#define 	hrLINEAR				0x00
#define 	hrNONLINEAR				0x08
#define 	hrPRESTATE				0x00
#define 	hrNPRESTATE				0x10
#define 	hrNONULL				0x00
#define 	hrNULL					0x20
#define 	hrNVOLATILE				0x00
#define 	hrVOLATILE				0x40
#define 	hrBITFIELD				0x00
#define 	hrBUFFERED				0x80
#define hrCOLLECTION				0xa1
#define 	hrcPHYSICAL				0x00
#define 	hrcAPPLICATION			0x01
#define 	hrcLOGICAL				0x02
#define 	hrcREPORT				0x03
#define 	hrcNARRAY				0x04
#define 	hrcUSWITCH				0x05
#define 	hrcUMODIFIER			0x06
#define 	hrcVENDOR				0x80
#define hrENDCOLLECT				0xC0
#define hrUPAGE						0x05
#define hrVUPAGE					0x06
#define 	hruGDESKTOP				0x01
#define 	hruSIMCONTROL			0x02
#define 	hruVRCONTROL			0x03
#define 	hruSPORTCONTROL			0x04
#define 	hruGAMECONTROL			0x05
#define 	hruGENCONTROL			0x06
#define 	hruKEYBOARD				0x07
#define 	hruLED					0x08
#define 	hruBUTTON				0x09
#define 	hruORDINAL				0x0A
#define 	hruTELEPHONY			0x0B
#define 	hruCONSUMER				0x0C
#define 	hruDIGITIZER			0x0D
#define 	hruPIDPAGE				0x0F
#define 	hruUNICODE				0x10
#define 	hruALPHANUM				0x14
#define 	hruMEDICAL				0x40
#define 	hruMONITOR				0x80
#define 	hruPOWER				0x84
#define 	hruBARCODE				0x8C
#define 	hruSCALE				0x8D
#define 	hruMAGNETIC				0x8E
#define 	hruPOS					0x8F
#define 	hruCAMERA				0x90
#define hrUMIN						0x19
#define hrUMAX						0x29
#define hrLMIN						0x15
#define hrLMINex					0x16
#define hrLMAX						0x25
#define hrLMAXex					0x26
#define hrRCOUNT					0x95
#define hrRSIZE						0x75
#define hrUNITS						0x67
#define hrUEXPS						0x55

#define hrUSAGE						0x09

/* generic desktop usage identificators */
#define 	hrgdPOINTER				0x01
#define 	hrgdMOUSE				0x02
#define 	hrgdJOYSTICK			0x04
#define 	hrgdGAMEPAD				0x05
#define 	hrgdKEYBOARD			0x06
#define 	hrgdKEYPAD				0x07
#define 	hrgdMAXISCONTROL		0x08
#define 	hrgdX					0x30
#define 	hrgdY					0x31
#define 	hrgdZ					0x32
#define 	hrgdRX					0x33
#define 	hrgdRY					0x34
#define 	hrgdRZ					0x35
#define 	hrgdSLIDER				0x36
#define 	hrgdDIAL				0x37
#define 	hrgdWHEEL				0x38
#define 	hrgdHATSWITCH			0x39
#define 	hrgdCOUNTBUFF			0x3A
#define 	hrgdBYTECOUNT			0x3B
#define 	hrgdMOTIONWAKE			0x3C
#define 	hrgdSTART				0x3D
#define 	hrgdSELECT				0x3E
#define 	hrgdVX					0x40
#define 	hrgdVY					0x41
#define 	hrgdVZ					0x42
#define 	hrgdVBRX				0x43
#define 	hrgdVBRY				0x44
#define 	hrgdVBRZ				0x45
#define 	hrgdVNO					0x46
#define 	hrgdFEATURENOTE			0x47
#define 	hrgdSYSCONTROL			0x80
#define 	hrgdSYSPOWERD			0x81
#define 	hrgdSYSSLEEP			0x82
#define 	hrgdSYSWAKE				0x83
#define 	hrgdSYSCONTEXT			0x84
#define 	hrgdSYSMAIN				0x85
#define 	hrgdSYSAPP				0x86
#define 	hrgdSYSHELP				0x87
#define 	hrgdSYSEXIT				0x88
#define 	hrgdSYSSELECT			0x89
#define 	hrgdSYSRIGHT			0x8A
#define 	hrgdSYSLEFT				0x8B
#define 	hrgdSYSUP				0x8C
#define 	hrgdSYSDOWN				0x8D
#define 	hrgdSYSCRESTART			0x8E
#define 	hrgdSYSWRESTART			0x8F
#define 	hrgdDPADUP				0x90
#define 	hrgdDPADDOWN			0x91
#define 	hrgdDPADRIGHT			0x92
#define 	hrgdDPADLEFT			0x93
#define 	hrgdSYSDOCK				0xA0
#define 	hrgdSYSUNDOCK			0xA1
#define 	hrgdSYSSETUP			0xA2
#define 	hrgdSYSBREAK			0xA3
#define 	hrgdSYSDEBUG			0xA4
#define 	hrgdAPPBREAK			0xA5
#define 	hrgdAPPDEBUG			0xA6
#define 	hrgdSYSMUTE				0xA7
#define 	hrgdSYSHIBERNATE		0xA8
#define 	hrgdSYSDINVERT			0xB0
#define 	hrgdSYSDINTERNAL		0xB1
#define 	hrgdSYSDEXTERNAL		0xB2
#define 	hrgdSYSDBOTH			0xB3
#define 	hrgdSYSDDUAL			0xB4
#define 	hrgdSYSDTOGGLE			0xB5
#define 	hrgdSYSDSWAP			0xB6
#define 	hrgdSYSDAUTOSCL			0xB7

/* LED usage identificators */
#define 	hrlNUMLOCK				0x01
#define 	hrlCAPSLOCK				0x02
#define 	hrlSCROLLLOCK			0x03
#define 	hrlCOMPOSE				0x04
#define 	hrlKANA					0x05
#define 	hrlPOWER				0x06
#define 	hrlSHIFT				0x07
#define 	hrlDNDISTURB			0x08
#define 	hrlMUTE					0x09
#define 	hrlTONEENA				0x0A
#define 	hrlHIFILT				0x0B
#define 	hrlLOFILT				0x0C
#define 	hrlEQENA				0x0D
#define 	hrlSOUNDFIELD			0x0E
#define 	hrlSURROUND				0x0F
#define 	hrlREPEAT				0x10
#define 	hrlSTEREO				0x11
#define 	hrlSRDETECT				0x12
#define 	hrlSPINNING				0x13
#define 	hrlCAV					0x14
#define 	hrlCLV					0x15
#define 	hrlFORMATDETECT			0x16
#define 	hrlOFFHOOK				0x17
#define 	hrlRING					0x18
#define 	hrlMESSAGE				0x19
#define 	hrlDATAMODE				0x1A
#define 	hrlBATTERYOP			0x1B
#define 	hrlBATTOK				0x1C
#define 	hrlBATTLOW				0x1D
#define 	hrlSPEAKER				0x1E
#define 	hrlHEADSET				0x1F
#define 	hrlHOLD					0x20
#define 	hrlMICROPHONE			0x21
#define 	hrlCOVERAGE				0x22
#define 	hrlNIGHTMODE			0x23
#define 	hrlSENDCALLS			0x24
#define 	hrlCALLPICKUP			0x25
#define 	hrlCONFERENCE			0x26
#define 	hrlSTANDBY				0x27
#define 	hrlCAMERAON				0x28
#define 	hrlCAMERAOFF			0x29
#define 	hrlONLINE				0x2A
#define 	hrlOFFLINE				0x2B
#define 	hrlBUSY					0x2C
#define 	hrlREADY				0x2D
#define 	hrlPAPEROUT				0x2E
#define 	hrlPAPERJAM				0x2F
#define 	hrlREMOTE				0x30
#define 	hrlFORWARD				0x31
#define 	hrlREVERSE				0x32
#define 	hrlSTOP					0x33
#define 	hrlREWIND				0x34
#define 	hrlFASTFORARD			0x35
#define 	hrlPLAY					0x36
#define 	hrlPAUSE				0x37
#define 	hrlRECORD				0x38
#define 	hrlERROR				0x39
#define 	hrlSELINDICATOR			0x3A
#define 	hrlINUSE				0x3B
#define 	hrlMULTIMODE			0x3C
#define 	hrlINDON				0x3D
#define 	hrlINDFLASH				0x3E
#define 	hrlINDSLOWBLINK			0x3F
#define 	hrlINDFASTBLINK			0x40
#define 	hrlINDOFFSEL			0x41
#define 	hrlFLASHON				0x42
#define 	hrlSLOWBLINKON			0x43
#define 	hrlSLOWBLINKOFF			0x44
#define 	hrlFASTBLINKON			0x45
#define 	hrlFASTBLINKOFF			0x46
#define 	hrlINDCOLOR				0x47
#define 	hrlINDRED				0x48
#define 	hrlINDGREEN				0x49
#define 	hrlINDAMBER				0x4A
#define 	hrlINDGENERIC			0x4B
#define 	hrlSYSSUSPEND			0x4C
#define 	hrlEXTPOWER				0x4D



/**
  * @}
  */ 


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */


/**
  * @}
  */ 



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

extern USBD_Class_cb_TypeDef  USBD_HID_cb;
extern uint8_t Buffer[];
/**
  * @}
  */ 

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */ 
uint8_t USBD_HID_SendReport (USB_OTG_CORE_HANDLE  *pdev, 
                                 uint8_t *report,
                                 uint16_t len);
/**
  * @}
  */ 

#endif  // __USB_HID_CORE_H_
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
