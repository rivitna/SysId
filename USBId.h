/***************************************************************************/
/* USBId.C - ������������� ��������� USB                                   */
/* ������ 1.00 (������� 2012 �.)                                           */
/*                                                                         */
/* OS: Win32                                                               */
/*                                                                         */
/* Copyright (c) 2002-2012 rivitna                                         */
/***************************************************************************/
//---------------------------------------------------------------------------
#ifndef __USBID_H__
#define __USBID_H__
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
//---------------------------------------------------------------------------
// ������������ ����� ������ ���������� USB (�� ����� DDK "usb100.h")
#define MAXIMUM_USB_STRING_LENGTH  255

// ���������� ���������� USB (�� ����� DDK "usb100.h")
#pragma pack(push, 1)
typedef struct _USB_DEVICE_DESCRIPTOR
{
  UCHAR   bLength;
  UCHAR   bDescriptorType;
  USHORT  bcdUSB;
  UCHAR   bDeviceClass;
  UCHAR   bDeviceSubClass;
  UCHAR   bDeviceProtocol;
  UCHAR   bMaxPacketSize0;
  USHORT  idVendor;
  USHORT  idProduct;
  USHORT  bcdDevice;
  UCHAR   iManufacturer;
  UCHAR   iProduct;
  UCHAR   iSerialNumber;
  UCHAR   bNumConfigurations;
} USB_DEVICE_DESCRIPTOR, *PUSB_DEVICE_DESCRIPTOR;
#pragma pack(pop)

// ����� ��������� ���������� USB
// ��: Windows XP � ����
BOOL FindUSBDiskDevice(
  IN TCHAR chDevice,
  OUT HANDLE *phHubDevice,
  OUT UINT *pnPortNum
  );

// ��������� ����������� ���������� USB
// ��: Windows 2000 � ����
BOOL GetUSBDeviceDescriptor(
  IN HANDLE hHubDevice,
  IN int nPortNum,
  OUT PUSB_DEVICE_DESCRIPTOR pDevDesc
  );

// ��������� ������ ���������� USB
// ��: Windows 2000 � ����
BOOL GetUSBStringW(
  IN HANDLE hHubDevice,
  IN int nPortNum,
  IN int nDescIndex,
  IN USHORT languageID,
  OUT LPWSTR pBuffer,
  IN OUT UINT *pnSize
  );
BOOL GetUSBStringA(
  IN HANDLE hHubDevice,
  IN int nPortNum,
  IN int nDescIndex,
  IN USHORT languageID,
  OUT LPSTR pBuffer,
  IN OUT UINT *pnSize
  );
#ifdef _UNICODE
#define GetUSBString  GetUSBStringW
#else
#define GetUSBString  GetUSBStringA
#endif  // _UNICODE

// ��������� ��������� ������ ��������� ���������� USB
// ��: Windows XP � ����
BOOL GetUSBDeviceSerialNumber(
  IN TCHAR chDevice,
  OUT LPTSTR pBuffer,
  IN OUT UINT *pnSize
  );
//---------------------------------------------------------------------------
#ifdef __cplusplus
}  // extern "C"
#endif // __cplusplus
//---------------------------------------------------------------------------
#endif  // __USBID_H__
