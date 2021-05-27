/***************************************************************************/
/* USBId.C - Идентификация устройств USB                                   */
/* Версия 1.00 (Октябрь 2012 г.)                                           */
/*                                                                         */
/* OS: Win32                                                               */
/*                                                                         */
/* Copyright (c) 2002-2012 rivitna                                         */
/***************************************************************************/
//---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <tchar.h>
#include <setupapi.h>

#ifdef _USE_STDLIB
#include <memory.h>
#else
#include "StdUtils.h"
#endif  // _USE_STDLIB

#include "USBId.h"
//---------------------------------------------------------------------------
#ifndef countof
#define countof(a)  (sizeof(a)/sizeof(a[0]))
#endif  // countof

#ifndef offsetof
#define offsetof(a, b)  ((size_t)&(((a *)0)->b))
#endif  // offsetof
//---------------------------------------------------------------------------
#pragma comment(lib, "SetupAPI.lib")
//---------------------------------------------------------------------------
//
// Структуры и определения из файла "WinIoCtl.h"
//

#ifndef IOCTL_STORAGE_GET_DEVICE_NUMBER
#define IOCTL_STORAGE_GET_DEVICE_NUMBER    0x002D1080
#endif  // IOCTL_STORAGE_GET_DEVICE_NUMBER

typedef struct _STORAGE_DEVICE_NUMBER
{
  DWORD  DeviceType;       // The FILE_DEVICE_XXX type for this device
  DWORD  DeviceNumber;     // The number of this device
  DWORD  PartitionNumber;  // If the device is partitionable,
                           // the partition number of the device.
                           // Otherwise -1
} STORAGE_DEVICE_NUMBER, *PSTORAGE_DEVICE_NUMBER;

//
// Структуры и определения из файла DDK "cfgmgr32.h"
//

#define MAX_DEVICE_ID_LEN    200

#define CMAPI    __declspec(dllimport)

// Standardized Return Value data type
typedef DWORD  CONFIGRET;

// Device Instance Handle data type
typedef DWORD  DEVINST, *PDEVINST;

CMAPI CONFIGRET WINAPI CM_Get_Device_IDA(
  IN  DEVINST  dnDevInst,
  OUT PCHAR    Buffer,
  IN  ULONG    BufferLen,
  IN  ULONG    ulFlags
);
CMAPI CONFIGRET WINAPI CM_Get_Device_IDW(
  IN  DEVINST  dnDevInst,
  OUT PWCHAR   Buffer,
  IN  ULONG    BufferLen,
  IN  ULONG    ulFlags
);
typedef CMAPI CONFIGRET (WINAPI *PFNCM_GET_DEVICE_ID)(
  IN  DEVINST  dnDevInst,
  OUT PTCHAR   Buffer,
  IN  ULONG    BufferLen,
  IN  ULONG    ulFlags
  );
#ifdef _UNICODE
#define CM_Get_Device_ID    CM_Get_Device_IDW
#else
#define CM_Get_Device_ID    CM_Get_Device_IDA
#endif  // _UNICODE

CMAPI CONFIGRET WINAPI CM_Get_Parent(
  OUT PDEVINST pdnDevInst,
  IN  DEVINST  dnDevInst,
  IN  ULONG    ulFlags
);
typedef CMAPI CONFIGRET (WINAPI *PFNCM_GET_PARENT)(
  OUT PDEVINST pdnDevInst,
  IN  DEVINST  dnDevInst,
  IN  ULONG    ulFlags
  );

#define CR_SUCCESS    (0x00000000)

//
// Структуры и определения из файлов DDK "usbioctl.h", "usb100.h"
//

#define IOCTL_USB_GET_NODE_INFORMATION                   0x00220408
#define IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX     0x00220448
#define IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME     0x00220420
#define IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION    0x00220410

#define USB_DEVICE_DESCRIPTOR_TYPE           0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE    0x02
#define USB_STRING_DESCRIPTOR_TYPE           0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE        0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE         0x05

#pragma pack(push, 1)

typedef enum _USB_HUB_NODE
{
  UsbHub,
  UsbMIParent
} USB_HUB_NODE;

typedef struct _USB_HUB_DESCRIPTOR
{
  UCHAR   bDescriptorLength;    // Length of this descriptor
  UCHAR   bDescriptorType;      // Hub configuration type
  UCHAR   bNumberOfPorts;       // number of ports on this hub
  USHORT  wHubCharacteristics;  // Hub Charateristics
  UCHAR   bPowerOnToPowerGood;  // port power on till power good in 2ms
  UCHAR   bHubControlCurrent;   // max current in mA
  // room for 255 ports power control and removable bitmask
  UCHAR   bRemoveAndPowerMask[64];
} USB_HUB_DESCRIPTOR, *PUSB_HUB_DESCRIPTOR;

typedef struct _USB_HUB_INFORMATION
{
  USB_HUB_DESCRIPTOR  HubDescriptor;
  BOOLEAN             HubIsBusPowered;
} USB_HUB_INFORMATION, *PUSB_HUB_INFORMATION;

typedef struct _USB_MI_PARENT_INFORMATION
{
  ULONG NumberOfInterfaces;
} USB_MI_PARENT_INFORMATION, *PUSB_MI_PARENT_INFORMATION;

typedef struct _USB_NODE_INFORMATION
{
  ULONG  NodeType;  // (The USB_HUB_NODE type) hub, mi parent
  union
  {
    USB_HUB_INFORMATION        HubInformation;
    USB_MI_PARENT_INFORMATION  MiParentInformation;
  } u;
} USB_NODE_INFORMATION, *PUSB_NODE_INFORMATION;

typedef struct _USB_ENDPOINT_DESCRIPTOR
{
  UCHAR   bLength;
  UCHAR   bDescriptorType;
  UCHAR   bEndpointAddress;
  UCHAR   bmAttributes;
  USHORT  wMaxPacketSize;
  UCHAR   bInterval;
} USB_ENDPOINT_DESCRIPTOR, *PUSB_ENDPOINT_DESCRIPTOR;

typedef struct _USB_STRING_DESCRIPTOR
{
  UCHAR  bLength;
  UCHAR  bDescriptorType;
  WCHAR  bString[1];
} USB_STRING_DESCRIPTOR, *PUSB_STRING_DESCRIPTOR;

typedef enum _USB_CONNECTION_STATUS
{
  NoDeviceConnected,
  DeviceConnected,
  // failure codes, these map to fail reasons
  DeviceFailedEnumeration,
  DeviceGeneralFailure,
  DeviceCausedOvercurrent,
  DeviceNotEnoughPower,
  DeviceNotEnoughBandwidth,
  DeviceHubNestedTooDeeply,
  DeviceInLegacyHub
} USB_CONNECTION_STATUS, *PUSB_CONNECTION_STATUS;

typedef struct _USB_PIPE_INFO
{
  USB_ENDPOINT_DESCRIPTOR  EndpointDescriptor;
  ULONG                    ScheduleOffset;
} USB_PIPE_INFO, *PUSB_PIPE_INFO;

typedef struct _USB_NODE_CONNECTION_INFORMATION_EX
{
  ULONG                  ConnectionIndex;
  // usb device descriptor returned by this device during enumeration
  USB_DEVICE_DESCRIPTOR  DeviceDescriptor;
  UCHAR                  CurrentConfigurationValue;
  UCHAR                  Speed;
  BOOLEAN                DeviceIsHub;
  USHORT                 DeviceAddress;
  ULONG                  NumberOfOpenPipes;
  ULONG                  ConnectionStatus;  // The USB_CONNECTION_STATUS type
  USB_PIPE_INFO PipeList[0];
} USB_NODE_CONNECTION_INFORMATION_EX, *PUSB_NODE_CONNECTION_INFORMATION_EX;

typedef struct _USB_NODE_CONNECTION_DRIVERKEY_NAME
{
  ULONG  ConnectionIndex;   // INPUT
  ULONG  ActualLength;      // OUTPUT
  // unicode name for the devnode
  WCHAR  DriverKeyName[1];  // OUTPUT
} USB_NODE_CONNECTION_DRIVERKEY_NAME, *PUSB_NODE_CONNECTION_DRIVERKEY_NAME;

typedef struct _USB_DESCRIPTOR_REQUEST
{
  ULONG  ConnectionIndex;
  struct
  {
    UCHAR   bmRequest;
    UCHAR   bRequest;
    USHORT  wValue;
    USHORT  wIndex;
    USHORT  wLength;
  } SetupPacket;
  UCHAR  Data[0];
} USB_DESCRIPTOR_REQUEST, *PUSB_DESCRIPTOR_REQUEST;

#pragma pack(pop)
//---------------------------------------------------------------------------
/***************************************************************************/
/* GetDeviceNumber - Получение номера устройства                           */
/*                   ОС: Windows XP и выше                                 */
/***************************************************************************/
DWORD GetDeviceNumber(
  IN LPCTSTR pszDevicePath
  )
{
  HANDLE hDev;
  DWORD dwDevNum;
  STORAGE_DEVICE_NUMBER sdn;
  DWORD dwBytesReturned;

  hDev = CreateFile(pszDevicePath, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
  if (hDev == INVALID_HANDLE_VALUE)
    return (DWORD)-1;

  dwDevNum = (DWORD)-1;

  ZeroMemory(&sdn, sizeof(sdn));
  if (DeviceIoControl(hDev,
                      IOCTL_STORAGE_GET_DEVICE_NUMBER,
                      NULL, 0,
                      &sdn, sizeof(sdn),
                      &dwBytesReturned, NULL))
    dwDevNum = MAKELONG(sdn.DeviceNumber, sdn.DeviceType);

  CloseHandle(hDev);
  return dwDevNum;
}
/***************************************************************************/
/* GetUSBHubPortCount - Получение количества портов USB-хаба               */
/*                      ОС: Windows 2000 и выше                            */
/***************************************************************************/
UINT GetUSBHubPortCount(
  IN HANDLE hHubDevice
  )
{
  USB_NODE_INFORMATION hubInfo;
  DWORD dwBytesReturned;

  ZeroMemory(&hubInfo, sizeof(hubInfo));
  hubInfo.NodeType = UsbHub;
  if (DeviceIoControl(hHubDevice,
                      IOCTL_USB_GET_NODE_INFORMATION,
                      &hubInfo, sizeof(hubInfo),
                      &hubInfo, sizeof(hubInfo),
                      &dwBytesReturned, NULL))
    return hubInfo.u.HubInformation.HubDescriptor.bNumberOfPorts;
  return 0;
}
/***************************************************************************/
/* IsUSBDeviceConnected - Проверка, подключено ли к порту устройство USB   */
/*                        ОС: Windows 2000 и выше                          */
/***************************************************************************/
BOOL IsUSBDeviceConnected(
  IN HANDLE hHubDevice,
  IN int nPortNum
  )
{
  USB_NODE_CONNECTION_INFORMATION_EX connectionInfoEx;
  DWORD dwBytesReturned;

  ZeroMemory(&connectionInfoEx, sizeof(connectionInfoEx));
  connectionInfoEx.ConnectionIndex = nPortNum;
  return (DeviceIoControl(hHubDevice,
                          IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
                          &connectionInfoEx, sizeof(connectionInfoEx),
                          &connectionInfoEx, sizeof(connectionInfoEx),
                          &dwBytesReturned, NULL) &&
          (connectionInfoEx.ConnectionStatus == DeviceConnected));
}
/***************************************************************************/
/* CheckUSBDeviceInstanceID - Проверка идентификатора экземпляра           */
/*                            устройства USB                               */
/*                            ОС: Windows 2000 и выше                      */
/***************************************************************************/
BOOL CheckUSBDeviceInstanceID(
  IN HANDLE hHubDevice,
  IN int nPortNum,
  IN LPCTSTR pszInstanceID
  )
{
  // Наименование перечислителя устройств USB
  const TCHAR USB_DEVICE_ENUM[] = _T("USB");

  BYTE driverKeyName[sizeof(USB_NODE_CONNECTION_DRIVERKEY_NAME) +
                     MAXIMUM_USB_STRING_LENGTH * sizeof(WCHAR)];
  PUSB_NODE_CONNECTION_DRIVERKEY_NAME pDriverKeyName;
  DWORD dwBytesReturned;
  HDEVINFO hDevInfo;
  BOOL bSuccess;
  BYTE buf[(MAXIMUM_USB_STRING_LENGTH + 1) * sizeof(WCHAR)];
  SP_DEVINFO_DATA da;
  UINT i;

  // Получение имени ключа реестра драйвера
  pDriverKeyName = (PUSB_NODE_CONNECTION_DRIVERKEY_NAME)&driverKeyName;
  ZeroMemory(&driverKeyName, sizeof(driverKeyName));
  pDriverKeyName->ConnectionIndex = nPortNum;
  if (!DeviceIoControl(hHubDevice,
                       IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
                       &driverKeyName, sizeof(driverKeyName),
                       &driverKeyName, sizeof(driverKeyName),
                       &dwBytesReturned, NULL))
    return FALSE;

  // Поиск устройства по имени ключа реестра драйвера
  hDevInfo = SetupDiGetClassDevs(NULL, USB_DEVICE_ENUM, NULL,
                                 DIGCF_PRESENT | DIGCF_ALLCLASSES);
  if (hDevInfo == INVALID_HANDLE_VALUE)
    return FALSE;

  bSuccess = FALSE;
  da.cbSize = sizeof(SP_DEVINFO_DATA);
  for (i = 0; ; i++)
  {
    if (!SetupDiEnumDeviceInfo(hDevInfo, i, &da))
      break;
    if (SetupDiGetDeviceRegistryPropertyW(hDevInfo, &da, SPDRP_DRIVER,
                                          NULL, buf, sizeof(buf), NULL))
    {
      // Сравнение имен ключа реестра драйвера
      if (!lstrcmpW((WCHAR *)buf, pDriverKeyName->DriverKeyName))
      {
        // Получение идентификатора экземпляра устройства
        if (SetupDiGetDeviceInstanceId(hDevInfo, &da, (LPTSTR)buf,
                                       sizeof(buf) / sizeof(TCHAR), NULL))
        {
          // Сравнение идентификаторов экземпляра устройства
          if (!lstrcmp((LPTSTR)buf, pszInstanceID)) bSuccess = TRUE;
        }
        break;
      }
    }
  }
  SetupDiDestroyDeviceInfoList(hDevInfo);
  return bSuccess;
}
/***************************************************************************/
/* FindUSBDiskDevice - Поиск дискового устройства USB                      */
/*                     ОС: Windows XP и выше                               */
/***************************************************************************/

#define BUFFER_SIZE  512  // >= MAX_DEVICE_ID_LEN (200)

// Замена символов в строке
void ReplaceChar(
  IN OUT TCHAR *s,
  IN TCHAR chOld,
  IN TCHAR chNew
  )
{
  while (*s != _T('\0'))
  {
    if (*s == chOld) *s = chNew;
    s++;
  }
}

BOOL FindUSBDiskDevice(
  IN TCHAR chDevice,
  OUT HANDLE *phHubDevice,
  OUT UINT *pnPortNum
  )
{
  // GUID для класса интерфейса дисковых устройств
  const GUID GUID_DEVINTERFACE_DISK = { 0x53f56307L, 0xb6bf, 0x11d0,
                                        { 0x94, 0xf2, 0x00, 0xa0,
                                          0xc9, 0x1e, 0xfb, 0x8b } };
  // Строка GUID для класса интерфейса устройств USB-хаб
  const TCHAR GUID_DEVINTERFACE_USB_HUB_STR[] =
    _T("f18a0e88-c30c-11d0-8815-00a0c906bed8");
  // Сигнатура идентификатора экземпляра устройства USB
#ifdef _UNICODE
  const DWORD USB_DEVICE_INSTANCE_ID_SIGN1 = 0x00530055;  // 'US'
  const DWORD USB_DEVICE_INSTANCE_ID_SIGN2 = 0x005C0042;  // 'B\\'
#else
  const DWORD USB_DEVICE_INSTANCE_ID_SIGN = 0x5C425355;   // 'USB\\'
#endif  // _UNICODE

  HINSTANCE hSetupAPIDll;
  PFNCM_GET_DEVICE_ID pfnCM_Get_Device_ID;
  PFNCM_GET_PARENT pfnCM_Get_Parent;

  DWORD dwDeviceNumber;
  DEVINST devInst = 0;
  TCHAR szInstanceID[MAX_DEVICE_ID_LEN];
  HANDLE hHubDevice;
  BYTE buf[BUFFER_SIZE];
  TCHAR szDevicePath[7];
  HDEVINFO hDevInfo;
  SP_DEVICE_INTERFACE_DATA dia;
  SP_DEVINFO_DATA da;
  PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd;
  DWORD cbdidd;
  UINT i;
  TCHAR szHubDevPath[(sizeof(buf) / sizeof(TCHAR) - 1) +
                     (countof(GUID_DEVINTERFACE_USB_HUB_STR) - 1) + 8];
  TCHAR *pch;
  UINT cch;
  UINT nHubPortCount;

  // Получение дескриптора SETUPAPI.DLL
  hSetupAPIDll = GetModuleHandle(_T("SETUPAPI.DLL"));
  if (hSetupAPIDll == NULL)
    return FALSE;
  // Получение адресов необходимых функций в SETUPAPI.DLL
#ifdef _UNICODE
  *(FARPROC *)&pfnCM_Get_Device_ID = GetProcAddress(hSetupAPIDll,
                                                    "CM_Get_Device_IDW");
#else
  *(FARPROC *)&pfnCM_Get_Device_ID = GetProcAddress(hSetupAPIDll,
                                                    "CM_Get_Device_IDA");
#endif  // _UNICODE
  *(FARPROC *)&pfnCM_Get_Parent = GetProcAddress(hSetupAPIDll,
                                                 "CM_Get_Parent");
  if ((pfnCM_Get_Device_ID == NULL) ||
      (pfnCM_Get_Parent == NULL))
    return SetLastError(ERROR_PROC_NOT_FOUND), FALSE;

  // Получение номера устройства
  szDevicePath[0] = szDevicePath[1] = szDevicePath[3] = _T('\\');
  szDevicePath[2] = _T('.');
  szDevicePath[4] = chDevice;
  szDevicePath[5] = _T(':');
  szDevicePath[6] = _T('\0');
  dwDeviceNumber = GetDeviceNumber(szDevicePath);
  if (dwDeviceNumber == (DWORD)-1)
    return FALSE;

  // Поиск устройства по номеру устройства
  hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_DISK, NULL, NULL,
                                 DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (hDevInfo == INVALID_HANDLE_VALUE)
    return FALSE;

  dia.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
  da.cbSize = sizeof(SP_DEVINFO_DATA);
  pdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)&buf;
  pdidd->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
  cbdidd = sizeof(buf);
  for (i = 0; ; i++)
  {
    if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_DISK,
                                     i, &dia))
      break;
    if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &dia, pdidd, cbdidd,
                                        NULL, &da))
    {
      // Сравнение номеров устройств
      if (dwDeviceNumber == GetDeviceNumber(pdidd->DevicePath))
      {
        devInst = da.DevInst;
        break;
      }
    }
  }
  SetupDiDestroyDeviceInfoList(hDevInfo);
  if (devInst == 0)
    return FALSE;

  // Получение идентификатора экземпляра устройства
  if ((CR_SUCCESS != pfnCM_Get_Parent(&devInst, devInst, 0L)) ||
      (CR_SUCCESS != pfnCM_Get_Device_ID(devInst, szInstanceID,
                                         countof(szInstanceID), 0L)))
    return FALSE;
  // Проверка сигнатуры идентификатора экземпляра устройства USB
#ifdef _UNICODE
  if ((lstrlen(szInstanceID) < (2 * sizeof(DWORD)) / sizeof(WCHAR)) ||
      (((DWORD *)szInstanceID)[0] != USB_DEVICE_INSTANCE_ID_SIGN1) ||
      (((DWORD *)szInstanceID)[1] != USB_DEVICE_INSTANCE_ID_SIGN2))
    return FALSE;
#else
  if ((lstrlen(szInstanceID) < sizeof(DWORD)) ||
      (*((DWORD *)szInstanceID) != USB_DEVICE_INSTANCE_ID_SIGN))
    return FALSE;
#endif  // _UNICODE
  // Получение пути к устройству USB-хаб
  if ((CR_SUCCESS != pfnCM_Get_Parent(&devInst, devInst, 0L)) ||
      (CR_SUCCESS != pfnCM_Get_Device_ID(devInst, (TCHAR *)buf,
                                         sizeof(buf) / sizeof(TCHAR), 0L)))
    return FALSE;
  pch = szHubDevPath;
  *pch++ = _T('\\');
  *pch++ = _T('\\');
  *pch++ = _T('.');
  *pch++ = _T('\\');
  ReplaceChar((TCHAR *)buf, _T('\\'), _T('#'));
  cch = lstrlen((LPTSTR)buf);
  memcpy(pch, buf, cch * sizeof(TCHAR));
  pch += cch;
  *pch++ = _T('#');
  *pch++ = _T('{');
  memcpy(pch, GUID_DEVINTERFACE_USB_HUB_STR,
         (countof(GUID_DEVINTERFACE_USB_HUB_STR) - 1) * sizeof(TCHAR));
  pch += countof(GUID_DEVINTERFACE_USB_HUB_STR) - 1;
  *pch++ = _T('}');
  *pch = _T('\0');
  // Получение дескриптора устройства USB-хаб
  hHubDevice = CreateFile(szHubDevPath, GENERIC_WRITE, FILE_SHARE_WRITE,
                          NULL, OPEN_EXISTING, 0, NULL);
  if (hHubDevice == INVALID_HANDLE_VALUE)
    return FALSE;

  // Поиск порта, к которому подключено устройство USB
  nHubPortCount = GetUSBHubPortCount(hHubDevice);
  for (i = 1; i <= nHubPortCount; i++)
  {
    if (IsUSBDeviceConnected(hHubDevice, i))
    {
      // Проверка идентификатора экземпляра устройства USB
      if (CheckUSBDeviceInstanceID(hHubDevice, i, szInstanceID))
      {
        *phHubDevice = hHubDevice;
        *pnPortNum = i;
        return TRUE;
      }
    }
  }

  CloseHandle(hHubDevice);
  return FALSE;
}
#undef BUFFER_SIZE
/***************************************************************************/
/* GetUSBDeviceDescriptor - Получение дескриптора устройства USB           */
/*                          ОС: Windows 2000 и выше                        */
/***************************************************************************/
BOOL GetUSBDeviceDescriptor(
  IN HANDLE hHubDevice,
  IN int nPortNum,
  OUT PUSB_DEVICE_DESCRIPTOR pDevDesc
  )
{
  BYTE devDescReq[sizeof(USB_DESCRIPTOR_REQUEST) +
                  sizeof(USB_DEVICE_DESCRIPTOR)];
  PUSB_DESCRIPTOR_REQUEST pDescReq;
  DWORD dwBytesReturned;

  ZeroMemory(&devDescReq, sizeof(devDescReq));
  pDescReq = (PUSB_DESCRIPTOR_REQUEST)&devDescReq;
  pDescReq->ConnectionIndex = nPortNum;
  pDescReq->SetupPacket.wValue = USB_DEVICE_DESCRIPTOR_TYPE << 8;
  pDescReq->SetupPacket.wLength = sizeof(USB_DEVICE_DESCRIPTOR);
  if (!DeviceIoControl(hHubDevice,
                       IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                       &devDescReq, sizeof(devDescReq),
                       &devDescReq, sizeof(devDescReq),
                       &dwBytesReturned, NULL))
    return FALSE;
  memcpy(pDevDesc, pDescReq + 1, sizeof(USB_DEVICE_DESCRIPTOR));
  return TRUE;
}
/***************************************************************************/
/* GetUSBStringW - Получение строки устройства USB                         */
/*                 ОС: Windows 2000 и выше                                 */
/***************************************************************************/
BOOL GetUSBStringW(
  IN HANDLE hHubDevice,
  IN int nPortNum,
  IN int nDescIndex,
  IN USHORT languageID,
  OUT LPWSTR pBuffer,
  IN OUT UINT *pnSize
  )
{
  BYTE strDescReq[sizeof(USB_DESCRIPTOR_REQUEST) +
                  sizeof(USB_STRING_DESCRIPTOR) +
                  MAXIMUM_USB_STRING_LENGTH * sizeof(WCHAR)];
  PUSB_DESCRIPTOR_REQUEST pDescReq;
  PUSB_STRING_DESCRIPTOR pStrDesc;
  DWORD dwBytesReturned;
  UINT cch;

  ZeroMemory(&strDescReq, sizeof(strDescReq));
  pDescReq = (PUSB_DESCRIPTOR_REQUEST)&strDescReq;
  pDescReq->ConnectionIndex = nPortNum;
  pDescReq->SetupPacket.wValue = MAKEWORD(nDescIndex,
                                          USB_STRING_DESCRIPTOR_TYPE);
  pDescReq->SetupPacket.wIndex = languageID;
  pDescReq->SetupPacket.wLength = sizeof(strDescReq) -
                                  sizeof(USB_DESCRIPTOR_REQUEST);
  pStrDesc = (PUSB_STRING_DESCRIPTOR)(pDescReq + 1);
  if (!DeviceIoControl(hHubDevice,
                       IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                       &strDescReq, sizeof(strDescReq),
                       &strDescReq, sizeof(strDescReq),
                       &dwBytesReturned, NULL))
    return FALSE;
  if ((dwBytesReturned < 2) ||
      (pStrDesc->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE) ||
      ((UINT)pStrDesc->bLength != dwBytesReturned -
                                  sizeof(USB_DESCRIPTOR_REQUEST)) ||
      (pStrDesc->bLength & 1))
    return SetLastError(ERROR_INVALID_DATA), FALSE;
  cch = (pStrDesc->bLength - offsetof(USB_STRING_DESCRIPTOR, bString)) /
        sizeof(WCHAR);
  if ((pBuffer == NULL) || (cch >= *pnSize))
  {
    *pnSize = cch + 1;
    return SetLastError(ERROR_INSUFFICIENT_BUFFER), FALSE;
  }
  memcpy(pBuffer, pStrDesc->bString, cch * sizeof(WCHAR));
  pBuffer[cch] = L'\0';
  *pnSize = cch;
  return TRUE;
}
/***************************************************************************/
/* GetUSBStringA - Получение строки устройства USB                         */
/*                 ОС: Windows 2000 и выше                                 */
/***************************************************************************/
BOOL GetUSBStringA(
  IN HANDLE hHubDevice,
  IN int nPortNum,
  IN int nDescIndex,
  IN USHORT languageID,
  OUT LPSTR pBuffer,
  IN OUT UINT *pnSize
  )
{
  WCHAR buf[MAXIMUM_USB_STRING_LENGTH + 1];
  UINT cch;
  UINT cb;

  // Получение строки устройства USB
  cch = countof(buf);
  if (!GetUSBStringW(hHubDevice, nPortNum, nDescIndex, languageID,
                     buf, &cch))
    return FALSE;
  cb = WideCharToMultiByte(CP_ACP, 0, buf, cch, NULL, 0, NULL, NULL);
  if ((pBuffer == NULL) || (cb >= *pnSize))
  {
    *pnSize = cb + 1;
    return SetLastError(ERROR_INSUFFICIENT_BUFFER), FALSE;
  }
  cb = WideCharToMultiByte(CP_ACP, 0, buf, cch, pBuffer, *pnSize, NULL,
                           NULL);
  pBuffer[cb] = '\0';
  *pnSize = cb;
  return TRUE;
}
/***************************************************************************/
/* GetUSBDeviceSerialNumber - Получение серийного номера дискового         */
/*                            устройства USB                               */
/*                            ОС: Windows XP и выше                        */
/***************************************************************************/
BOOL GetUSBDeviceSerialNumber(
  IN TCHAR chDevice,
  OUT LPTSTR pBuffer,
  IN OUT UINT *pnSize
  )
{
  HANDLE hHubDevice;
  UINT nPortNum;
  BOOL bSuccess;
  USB_DEVICE_DESCRIPTOR devDesc;
  DWORD dwError;

  // Поиск устройства USB, соответствующего указанному диску
  if (!FindUSBDiskDevice(chDevice, &hHubDevice, &nPortNum))
    return FALSE;

  bSuccess = FALSE;

  // Получение дескриптора устройства USB
  if (GetUSBDeviceDescriptor(hHubDevice, nPortNum, &devDesc))
  {
    // Получение строки с серийным номером устройства
    if (devDesc.iSerialNumber != 0)
    {
      if (GetUSBString(hHubDevice, nPortNum, devDesc.iSerialNumber, 0x409,
                       pBuffer, pnSize))
        bSuccess = TRUE;
    }
  }

  if (!bSuccess) dwError = GetLastError();
  CloseHandle(hHubDevice);
  if (!bSuccess)
    return SetLastError(dwError), FALSE;
  return TRUE;
}
//---------------------------------------------------------------------------
