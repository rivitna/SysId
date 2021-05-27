//---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <tchar.h>
#include <intrin.h>
#include <iphlpapi.h>

#ifdef _USE_STDLIB
#include <memory.h>
#else
#include "StdUtils.h"
#endif  // _USE_STDLIB

#include "DevId.h"
#include "USBId.h"
#include "StrUtils.h"
#include "AppUtils.h"
//---------------------------------------------------------------------------
#ifndef countof
#define countof(a)  (sizeof(a)/sizeof(a[0]))
#endif  // countof
//---------------------------------------------------------------------------
#ifndef _DEBUG
#pragma comment(linker, "/ENTRY:Start")
#pragma comment(linker, "/MERGE:.rdata=.text")
#pragma comment(linker, "/NODEFAULTLIB:libc.lib")
#pragma comment(linker, "/NODEFAULTLIB:libcmt.lib")
#endif  // _DEBUG
//---------------------------------------------------------------------------
// Максимальная длина строки файла отчета
#define MAX_LOG_LINE_LENGTH  80
// Смещение заголовка в строке файла отчета
#define LOG_TITLE_OFFSET  3
//---------------------------------------------------------------------------
// Инициализация файла отчета
BOOL InitLog();
// Запись заголовка раздела в файл отчета
void LogSectionTitle(
  LPCTSTR pszTitle
  );
// Запись заголовка в файл отчета
void LogTitle(
  LPCTSTR pszTitle
  );
// Запись форматированного заголовка в файл отчета
void LogFmtTitle(
  LPCTSTR pszFmt,
  ...
  );
//---------------------------------------------------------------------------
// Запись в файл отчета общей информации
void LogGeneralInfo();
// Запись в файл отчета информации об устройствах
void LogDevicesInfo();
// Запись в файл отчета информации о дисковых устройствах USB
void LogUSBDiskDevicesInfo();
// Запись в файл отчета информации о сетевых адаптерах
void LogNetAdaptersInfo();
// Запись в файл отчета информации о жестких дисках
void LogFixedDrivesInfo();
//---------------------------------------------------------------------------
/***************************************************************************/
/* main                                                                    */
/***************************************************************************/
#ifdef _DEBUG
int _tmain(int argc, TCHAR *argv[])
#else
int main(void)
#endif  // _DEBUG
{
  // Инициализация файла отчета
  if (!InitLog())
    return 1;
  // Запись в файл отчета общей информации
  LogGeneralInfo();
  // Запись в файл отчета информации об устройствах
  LogDevicesInfo();
  // Запись в файл отчета информации о дисковых устройствах USB
  LogUSBDiskDevicesInfo();
  // Запись в файл отчета информации о сетевых адаптерах
  LogNetAdaptersInfo();
  // Запись в файл отчета информации о жестких дисках
  LogFixedDrivesInfo();
  // Закрытие файла отчета
  CloseLog();
  return 0;
}
/***************************************************************************/
/* Start                                                                   */
/***************************************************************************/
#ifndef _DEBUG
void Start(void)
{
  int ret = main();
  ::ExitProcess((UINT)ret);
}
#endif  // _DEBUG
//---------------------------------------------------------------------------
/***************************************************************************/
/* InitLog - Инициализация файла отчета                                    */
/***************************************************************************/
BOOL InitLog()
{
  const TCHAR LOG_FILE_EXT[] = _T(".log");
  TCHAR szLogFileName[MAX_PATH + 1];

  // Получение параметра программы
  if (!GetArgument(::GetCommandLine(), 1, szLogFileName,
                   countof(szLogFileName)))
  {
    // Создание имени файла отчета
    if (!::GetModuleFileName(NULL, szLogFileName, countof(szLogFileName)))
      return FALSE;
    TCHAR *pFileName = StrRCharSet(szLogFileName, _T(":\\/"));
    if (pFileName != NULL)
    {
      if (*pFileName++ == _T(':')) *pFileName++ = _T('\\');
    }
    else
    {
      pFileName = szLogFileName;
    }
    // Получение имени компьютера
    DWORD cch = countof(szLogFileName) - (pFileName - szLogFileName);
    if (!::GetComputerName(pFileName, &cch))
      return FALSE;
    TCHAR *pFileExt = szLogFileName + ::lstrlen(szLogFileName);
    if (countof(LOG_FILE_EXT) > countof(szLogFileName) -
                                (pFileExt - szLogFileName))
      return FALSE;
    memcpy(pFileExt, LOG_FILE_EXT, sizeof(LOG_FILE_EXT));
  }

  // Открытие файла отчета
  return OpenLog(szLogFileName, FALSE);
}
/***************************************************************************/
/* LogSectionTitle - Запись заголовка раздела в файл отчета                */
/***************************************************************************/
void LogSectionTitle(
  LPCTSTR pszTitle
  )
{
  TCHAR line[MAX_LOG_LINE_LENGTH + 2 + 1];
  TCHAR buf[MAX_LOG_LINE_LENGTH + 2 + 1];

  int cchTitle = ::lstrlen(pszTitle);
  if (cchTitle <= MAX_LOG_LINE_LENGTH)
  {
    int nTitleOfs = (MAX_LOG_LINE_LENGTH - cchTitle) >> 1;
    StrSet(buf, _T(' '), nTitleOfs);
    memcpy(buf + nTitleOfs, pszTitle, cchTitle * sizeof(TCHAR));
    TCHAR *p = buf + nTitleOfs + cchTitle;
    *p++ = _T('\r');
    *p++ = _T('\n');
    *p = _T('\0');
  }

  StrSet(line, _T('*'), MAX_LOG_LINE_LENGTH);
  line[MAX_LOG_LINE_LENGTH] = _T('\r');
  line[MAX_LOG_LINE_LENGTH + 1] = _T('\n');
  line[MAX_LOG_LINE_LENGTH + 2] = _T('\0');

  Log(line);
  if (cchTitle <= MAX_LOG_LINE_LENGTH)
  {
    Log(buf);
  }
  else
  {
    Log(pszTitle);
    LogNewLine();
  }
  Log(line);
  LogNewLine();
}
/***************************************************************************/
/* LogTitle - Запись заголовка в файл отчета                               */
/***************************************************************************/
void LogTitle(
  LPCTSTR pszTitle
  )
{
  TCHAR buf[MAX_LOG_LINE_LENGTH + 2 + 1];

  int cchTitle = ::lstrlen(pszTitle);
  if (cchTitle > MAX_LOG_LINE_LENGTH - LOG_TITLE_OFFSET - 2)
  {
    StrSet(buf, _T('-'), LOG_TITLE_OFFSET);
    buf[LOG_TITLE_OFFSET] = _T(' ');
    buf[LOG_TITLE_OFFSET + 1] = _T('\0');
    Log(buf);
    Log(pszTitle);
    LogNewLine();
  }
  else
  {
    StrSet(buf, _T('-'), MAX_LOG_LINE_LENGTH);
    buf[MAX_LOG_LINE_LENGTH] = _T('\r');
    buf[MAX_LOG_LINE_LENGTH + 1] = _T('\n');
    buf[MAX_LOG_LINE_LENGTH + 2] = _T('\0');
    buf[LOG_TITLE_OFFSET] = buf[LOG_TITLE_OFFSET + cchTitle + 1] = _T(' ');
    memcpy(buf + LOG_TITLE_OFFSET + 1, pszTitle, cchTitle * sizeof(TCHAR));
    Log(buf);
  }
}
/***************************************************************************/
/* LogFmtTitle - Запись форматированного заголовка в файл отчета           */
/***************************************************************************/
void LogFmtTitle(
  LPCTSTR pszFmt,
  ...
  )
{
  TCHAR buf[1024];
  va_list arglist;
  va_start(arglist, pszFmt);
  ::wvsprintf(buf, pszFmt, arglist);
  va_end(arglist);
  // Запись заголовка в файл отчета
  return LogTitle(buf);
}
//---------------------------------------------------------------------------
/***************************************************************************/
/* MyGlobalMemoryStatusEx - Получение расширенной информации об            */
/*                          использовании системой физической и            */
/*                          виртуальной памяти                             */
/*                          ОС: Windows 2000 и выше                        */
/***************************************************************************/
BOOL MyGlobalMemoryStatusEx(
  LPMEMORYSTATUSEX lpBuffer
  )
{
  // Получение дескриптора KERNEL32.DLL
  HINSTANCE hKernel = ::GetModuleHandle(_T("KERNEL32.DLL"));
  if (hKernel == NULL)
    return FALSE;
  // Получение адреса функции GlobalMemoryStatusEx в KERNEL32.DLL
  BOOL (WINAPI *pfnGlobalMemoryStatusEx)(LPMEMORYSTATUSEX);
  *(FARPROC *)&pfnGlobalMemoryStatusEx =
    ::GetProcAddress(hKernel, "GlobalMemoryStatusEx");
  if (pfnGlobalMemoryStatusEx == NULL)
    return FALSE;
  return pfnGlobalMemoryStatusEx(lpBuffer);
}
/***************************************************************************/
/* IsWow64 - Проверка, запущен ли процесс из-под WOW64                     */
/***************************************************************************/
BOOL IsWow64()
{
  // Получение дескриптора KERNEL32.DLL
  HINSTANCE hKernel = ::GetModuleHandle(_T("KERNEL32.DLL"));
  if (hKernel == NULL)
    return FALSE;
  // Получение адреса функции IsWow64Process в KERNEL32.DLL
  BOOL (WINAPI *pfnIsWow64Process)(HANDLE, BOOL *);
  *(FARPROC *)&pfnIsWow64Process = ::GetProcAddress(hKernel,
                                                    "IsWow64Process");
  if (pfnIsWow64Process == NULL)
    return FALSE;
  BOOL bIsWow64;
  if (!pfnIsWow64Process(::GetCurrentProcess(), &bIsWow64))
    return FALSE;
  return bIsWow64;
}
/***************************************************************************/
/* GetPlatformName - Получение строки с наименованием платформы ОС         */
/***************************************************************************/
LPCTSTR GetPlatformName(
  DWORD dwPlatformId
  )
{
  switch (dwPlatformId)
  {
    case VER_PLATFORM_WIN32_WINDOWS:
      return _T("Windows 9x");
    case VER_PLATFORM_WIN32_NT:
      return _T("Windows NT");
  }
  return NULL;
}
/***************************************************************************/
/* LogGeneralInfo - Запись в файл отчета общей информации                  */
/***************************************************************************/
void LogGeneralInfo()
{
  TCHAR buf[256];
  DWORD cch;

  LogSectionTitle(_T("General Info"));

  // Получение версии ОС
  OSVERSIONINFO osvi;
  osvi.dwOSVersionInfoSize = sizeof(osvi);
  ::GetVersionEx(&osvi);

  // Получение имени компьютера
  cch = countof(buf);
  if (::GetComputerName(buf, &cch))
    LogFmt(_T("   Computer Name: %s\r\n"), buf);

  // Получение имени пользователя
  cch = countof(buf);
  if (::GetUserName(buf, &cch))
    LogFmt(_T("       User Name: %s\r\n"), buf);

  // Запись в файл отчета информации об ОС
  if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    osvi.dwBuildNumber &= 0xFFFF;
  LogFmt(_T("Operating System: %s x%u %u.%u.%u %s\r\n"),
         GetPlatformName(osvi.dwPlatformId),
         IsWow64() ? 64 : 32,
         osvi.dwMajorVersion,
         osvi.dwMinorVersion,
         osvi.dwBuildNumber,
         osvi.szCSDVersion);

  // Получение размера физической памяти
  unsigned __int64 nTotalPhys = 0;
  // Windows 2000 и выше?
  if ((osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
      (osvi.dwMajorVersion >= 5))
  {
    // Получение расширенной информации об использовании системой физической
    // и виртуальной памяти
    MEMORYSTATUSEX msex;
    msex.dwLength = sizeof(msex);
    if (MyGlobalMemoryStatusEx(&msex))
      nTotalPhys = msex.ullTotalPhys;
  }
  else
  {
    // Получение информации об использовании системой физической и
    // виртуальной памяти
    MEMORYSTATUS ms;
    ::GlobalMemoryStatus(&ms);
    nTotalPhys = ms.dwTotalPhys;
  }
  if (nTotalPhys != 0)
  {
    unsigned int nPhysMemInMB;
#ifdef _USE_STDLIB
    nPhysMemInMB = (unsigned int)((nTotalPhys + 0x80000) / 0x100000);
#else
    nPhysMemInMB = (unsigned int)DivUI64(nTotalPhys + 0x80000, 0x100000);
#endif  // _USE_STDLIB
    LogFmt(_T(" Physical Memory: %u MB\r\n"), nPhysMemInMB);
  }

  LogNewLine();
  LogNewLine();
}
//---------------------------------------------------------------------------
/***************************************************************************/
/* LogATADeviceInfo - Запись в файл отчета информации об устройстве        */
/*                    ATA/ATAPI                                            */
/***************************************************************************/
void LogATADeviceInfo(
  const ATA_DEVICE_INFO *pDevInfo
  )
{
  LPCTSTR pszDevice;
  TCHAR buf[32];
  // ATAPI-устройство?
  if (pDevInfo->wGenConfig & 0x8000)
  {
    pszDevice = ((pDevInfo->wGenConfig & 0x1F00) == 0x500)
                  ? _T("CD-ROM")
                  : _T("Unknown ATAPI");
  }
  else
  {
    // ATA-устройство
    // Определение размера ATA-устройства
    unsigned int nDevSizeInGB;
#ifdef _USE_STDLIB
    nDevSizeInGB = (unsigned int)GetATADeviceSize(pDevInfo) / 1000000000;
#else
    nDevSizeInGB = (unsigned int)DivUI64(GetATADeviceSize(pDevInfo),
                                         1000000000);
#endif  // _USE_STDLIB
    ::wsprintf(buf, _T("IDE HDD %u GB"), nDevSizeInGB);
    pszDevice = buf;
  }
  LogFmt(_T("           Device: %s\r\n"), pszDevice);
  if (*((WORD *)&pDevInfo->sModelNumber) != 0)
    LogFmt(_T("            Model: %.40hs\r\n"), pDevInfo->sModelNumber);
  if (*((WORD *)&pDevInfo->sFirmwareRev) != 0)
    LogFmt(_T("Firmware Revision: %.8hs\r\n"), pDevInfo->sFirmwareRev);
  if (*((WORD *)&pDevInfo->sSerialNumber) != 0)
    LogFmt(_T("    Serial Number: %.20hs\r\n"), pDevInfo->sSerialNumber);
}
/***************************************************************************/
/* LogDevicesInfo - Запись в файл отчета информации об устройствах         */
/***************************************************************************/
void LogDevicesInfo()
{
  BYTE buf[sizeof(ATA_DEVICE_INFO)];

  LogSectionTitle(_T("IDE/ATAPI/SCSI Devices"));

  // Получение контрольной суммы серийного номера жесткого диска
  if ((int)::GetVersion() < 0)  // Windows 9x?
  {
    const static
    struct
    {
      WORD     wBasePort;
      BYTE     bDevNum;
      LPCTSTR  pszName;
    } ataDeviceList[] =
    { { 0x1F0, 0, _T("Primary Master") },
      { 0x1F0, 1, _T("Primary Slave") },
      { 0x170, 0, _T("Secondary Master") },
      { 0x170, 1, _T("Secondary Slave") },
      { 0x1E8, 0, _T("Tertiary Master") },
      { 0x1E8, 1, _T("Tertiary Slave") },
      { 0x168, 0, _T("Quaternary Master") },
      { 0x168, 1, _T("Quaternary Slave") } };
    for (unsigned int i = 0; i < countof(ataDeviceList); i++)
    {
      // Идентификация устройства ATA/ATAPI (Windows 9x)
      if (Win9x_IdentifyDevice(ataDeviceList[i].wBasePort,
                               ataDeviceList[i].bDevNum,
                               (PATA_DEVICE_INFO)&buf))
      {
        LogTitle(ataDeviceList[i].pszName);
        // Запись в файл отчета информации об устройстве ATA/ATAPI
        LogATADeviceInfo((PATA_DEVICE_INFO)&buf);
        LogNewLine();
      }
    }
  }
  else  // Windows NT
  {
    for (unsigned int i = 0; i <= MAX_DEVICE_NUM; i++)
    {
      // Получение дескриптора физического устройства
      HANDLE hDevice = NtGetPhysicalDriveHandle((BYTE)i);
      if (hDevice == INVALID_HANDLE_VALUE)
        continue;

      // Идентификация устройства ATA/ATAPI (SMART)
      if (SMART_IdentifyDevice(hDevice, (BYTE)i, (PATA_DEVICE_INFO)&buf))
      {
        LogFmtTitle(_T("Device%u"), i);
        // Запись в файл отчета информации об устройстве ATA/ATAPI
        LogATADeviceInfo((PATA_DEVICE_INFO)&buf);
        LogNewLine();
      }
      else
      {
        // Получение информации о геометрии физического диска
        DISK_GEOMETRY diskGeometry;
        if (NtGetDiskGeometry(hDevice, &diskGeometry) &&
            (diskGeometry.MediaType == FixedMedia))
        {
          unsigned int nDevSizeInGB;
#ifdef _USE_STDLIB
          nDevSizeInGB = (unsigned int)(((diskGeometry.Cylinders.QuadPart *
                                          diskGeometry.TracksPerCylinder *
                                          diskGeometry.SectorsPerTrack) *
                                          diskGeometry.BytesPerSector) /
                                         1000000000);
#else
          nDevSizeInGB =
            (unsigned int)DivUI64(Mul64(Mul64(diskGeometry.Cylinders.QuadPart,
                                              __emulu(diskGeometry.TracksPerCylinder,
                                                      diskGeometry.SectorsPerTrack)),
                                        diskGeometry.BytesPerSector),
                                  1000000000);
#endif  // _USE_STDLIB
          LogFmtTitle(_T("Device%u"), i);
          BOOL bWriteSerialNumber = FALSE;
          // Получение серийного номера устройства SCSI
          unsigned int nSize = sizeof(buf) / sizeof(TCHAR);
          if (NtGetSCSIDeviceSerialNumber(hDevice, (LPTSTR)buf, &nSize) &&
              (nSize != 0))
          {
            bWriteSerialNumber = TRUE;
            LogFmt(_T("           Device: SCSI HDD %u GB\r\n"),
                   nDevSizeInGB);
          }
          else
          {
            LogFmt(_T("           Device: HDD %u GB\r\n"), nDevSizeInGB);
          }
          if (bWriteSerialNumber)
          {
            LogFmt(_T("    Serial Number: %s\r\n"), (LPTSTR)buf);
          }
          LogNewLine();
        }
      }

      ::CloseHandle(hDevice);
    }
  }
  LogNewLine();
}
//---------------------------------------------------------------------------
/***************************************************************************/
/* LogUSBDiskDeviceInfo - Запись в файл отчета информации о дисковом       */
/*                        устройстве USB                                   */
/*                        ОС: Windows XP и выше                            */
/***************************************************************************/
void LogUSBDiskDeviceInfo(
  TCHAR chDevice
  )
{
  HANDLE hHubDevice;
  unsigned int nPortNum;
  TCHAR buf[MAXIMUM_USB_STRING_LENGTH + 1];
  unsigned int cch;

  // Поиск устройства USB, соответствующего указанному диску
  if (!FindUSBDiskDevice(chDevice, &hHubDevice, &nPortNum))
    return;

  // Получение дескриптора устройства USB
  USB_DEVICE_DESCRIPTOR devDesc;
  if (GetUSBDeviceDescriptor(hHubDevice, nPortNum, &devDesc))
  {
    LogFmtTitle(_T("Drive %c"), chDevice & (~0x20));

    LogFmt(_T("    Vendor ID: %04X\r\n")
           _T("   Product ID: %04X\r\n"),
           devDesc.idVendor, devDesc.idProduct);
    if (devDesc.iManufacturer != 0)
    {
      cch = countof(buf);
      if (GetUSBString(hHubDevice, nPortNum, devDesc.iManufacturer, 0x409,
                       buf, &cch))
      {
        LogFmt(_T(" Manufacturer: %s\r\n"), buf);
      }
    }
    if (devDesc.iProduct != 0)
    {
      cch = countof(buf);
      if (GetUSBString(hHubDevice, nPortNum, devDesc.iProduct, 0x409,
                       buf, &cch))
      {
        LogFmt(_T("      Product: %s\r\n"), buf);
      }
    }
    if (devDesc.iSerialNumber != 0)
    {
      cch = countof(buf);
      if (GetUSBString(hHubDevice, nPortNum, devDesc.iSerialNumber, 0x409,
                       buf, &cch))
      {
        LogFmt(_T("Serial Number: %s\r\n"), buf);
      }
    }
    LogNewLine();
  }

  ::CloseHandle(hHubDevice);
}
/***************************************************************************/
/* LogUSBDiskDevicesInfo - Запись в файл отчета информации о дисковых      */
/*                         устройствах USB                                 */
/***************************************************************************/
void LogUSBDiskDevicesInfo()
{
  // Получение битовой карты доступных логических устройств
  DWORD dwLogicalDrives = ::GetLogicalDrives();
  if (dwLogicalDrives == 0)
    return;

  LogSectionTitle(_T("USB Disk Devices"));

  for (int i = 0; i < 26; i++)
  {
    if (!(dwLogicalDrives & (1 << i)))
      continue;
    // Запись в файл отчета информации о дисковом устройстве USB
    LogUSBDiskDeviceInfo(_T('A') + i);
  }
  LogNewLine();
}
//---------------------------------------------------------------------------
/***************************************************************************/
/* GetAdapterTypeStr - Получение строки с типом адаптера                   */
/***************************************************************************/
LPCTSTR GetAdapterTypeStr(
  unsigned int nAdapterType
  )
{
  switch (nAdapterType)
  {
    case MIB_IF_TYPE_OTHER:
      return _T("Other");
    case MIB_IF_TYPE_ETHERNET:
      return _T("Ethernet");
    case MIB_IF_TYPE_TOKENRING:
      return _T("Token Ring");
    case MIB_IF_TYPE_FDDI:
      return _T("FDDI");
    case MIB_IF_TYPE_PPP:
      return _T("PPP");
    case MIB_IF_TYPE_LOOPBACK:
      return _T("Loopback");
    case MIB_IF_TYPE_SLIP:
      return _T("Slip");
  }
  return _T("Unknown");
}
/***************************************************************************/
/* LogIPAddressList - Запись в файл отчета списка IP-адресов               */
/***************************************************************************/
void LogIPAddressList(
  PIP_ADDR_STRING pAddressList
  )
{
  PIP_ADDR_STRING pAddress = pAddressList;
  while (pAddress != NULL)
  {
    if (pAddress != pAddressList) Log(_T(", "));
    LogFmt(_T("%hs (%hs)"),
           pAddress->IpAddress.String,
           pAddress->IpMask.String);
    pAddress = pAddress->Next;
  }
  LogNewLine();
}
/***************************************************************************/
/* ConvertMACAddressToString - Преобразование MAC адреса в строку          */
/***************************************************************************/
void ConvertMACAddressToString(
  BYTE *pbMACAddress,
  unsigned int cbMACAddress,
  LPTSTR pBuffer
  )
{
  LPTSTR p = pBuffer;
  while (cbMACAddress != 0)
  {
    if (p != pBuffer) *p++ = _T('-');
    p += ::wsprintf(p, _T("%02X"), *pbMACAddress);
    pbMACAddress++;
    cbMACAddress--;
  }
  *p = _T('\0');
}
/***************************************************************************/
/* LogNetAdaptersInfo - Запись в файл отчета информации о сетевых адаптерах*/
/*                      ОС: Windows 98 и выше                              */
/***************************************************************************/
void LogNetAdaptersInfo()
{
  // Загрузка библиотеки IP Helper API
  HMODULE hIphlpapiLib = ::LoadLibrary(_T("IPHLPAPI.DLL"));
  if (hIphlpapiLib == NULL)
    return;

  DWORD (WINAPI *pfnGetAdaptersInfo)(IP_ADAPTER_INFO *, ULONG *);
  *(FARPROC *)&pfnGetAdaptersInfo = ::GetProcAddress(hIphlpapiLib,
                                                     "GetAdaptersInfo");
  if (pfnGetAdaptersInfo == NULL)
  {
    // Освобождение библиотеки IP Helper API
    ::FreeLibrary(hIphlpapiLib);
    ::SetLastError(ERROR_PROC_NOT_FOUND);
    return;
  }

  DWORD dwError;
  IP_ADAPTER_INFO adapterInfo;
  void *pAdapterInfoBuffer = NULL;
  IP_ADAPTER_INFO *pAdapterInfo;
  ULONG ulOutBufLen;

  pAdapterInfo = &adapterInfo;
  ulOutBufLen = sizeof(adapterInfo);
  // Получение информации о сетевых адаптерах
  dwError = pfnGetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
  if (dwError == ERROR_BUFFER_OVERFLOW)
  {
    // Выделение буфера для информации о сетевых адаптерах
    pAdapterInfoBuffer = ::HeapAlloc(::GetProcessHeap(), 0, ulOutBufLen);
    if (pAdapterInfoBuffer == NULL)
    {
      // Освобождение библиотеки IP Helper API
      ::FreeLibrary(hIphlpapiLib);
      ::SetLastError(ERROR_NOT_ENOUGH_MEMORY);
      return;
    }
    pAdapterInfo = (IP_ADAPTER_INFO *)pAdapterInfoBuffer;
    // Получение информации о сетевых адаптерах
    dwError = pfnGetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
  }

  // Освобождение библиотеки IP Helper API
  ::FreeLibrary(hIphlpapiLib);

  if (dwError != ERROR_SUCCESS)
  {
    if (pAdapterInfoBuffer != NULL)
      ::HeapFree(::GetProcessHeap(), 0, pAdapterInfoBuffer);
    ::SetLastError(dwError);
    return;
  }

  LogSectionTitle(_T("Network Adapters"));

  TCHAR buf[MAX_ADAPTER_ADDRESS_LENGTH * 3];
  unsigned int i = 0;
  do
  {
    LogFmtTitle(_T("Adapter%u"), i);
    LogFmt(_T("     Adapter: %hs\r\n")
           _T(" Description: %hs\r\n")
           _T("        Type: %s\r\n"),
           pAdapterInfo->AdapterName,
           pAdapterInfo->Description,
           GetAdapterTypeStr(pAdapterInfo->Type));
    // Вывод списка IP-адресов
    Log(_T("IP-addresses: "));
    LogIPAddressList(&pAdapterInfo->IpAddressList);
    // Преобразование MAC адреса в строку
    ConvertMACAddressToString(pAdapterInfo->Address,
                              pAdapterInfo->AddressLength,
                              buf);
    LogFmt(_T(" MAC-address: %s\r\n"), buf);
    LogNewLine();
    i++;
  } while ((pAdapterInfo = pAdapterInfo->Next) != NULL);
  LogNewLine();

  if (pAdapterInfoBuffer != NULL)
    ::HeapFree(::GetProcessHeap(), 0, pAdapterInfoBuffer);
}
//---------------------------------------------------------------------------
/***************************************************************************/
/* LogFixedDriveInfo - Запись в файл отчета информации о жестком диске     */
/***************************************************************************/
void LogFixedDriveInfo(
  LPCTSTR pszRootPath
  )
{
  // Получение информации о разделе
  TCHAR szVolumeName[MAX_PATH + 1];
  TCHAR szFSName[MAX_PATH + 1];
  DWORD dwVolumeSerialNumber;
  if (::GetVolumeInformation(pszRootPath,
                             szVolumeName, countof(szVolumeName),
                             &dwVolumeSerialNumber,
                             NULL,
                             NULL,
                             szFSName, countof(szFSName)))
  {
    LogFmt(_T("     Volume Label: %s\r\n")
           _T("    Serial Number: %04X-%04X\r\n")
           _T("      File System: %s\r\n"),
           szVolumeName,
           HIWORD(dwVolumeSerialNumber), LOWORD(dwVolumeSerialNumber),
           szFSName);
  }
  // Получение информации о размере свободного пространства на диске
  DWORD dwSectorsPerCluster;
  DWORD dwBytesPerSector;
  DWORD dwNumberOfFreeClusters;
  DWORD dwTotalNumberOfClusters;
  if (::GetDiskFreeSpace(pszRootPath, &dwSectorsPerCluster,
                         &dwBytesPerSector, &dwNumberOfFreeClusters,
                         &dwTotalNumberOfClusters))
  {
    DWORD dwBytesPerCluster = (DWORD)__emulu(dwBytesPerSector,
                                             dwSectorsPerCluster);
    TCHAR szTotalBytes[28];
    TCHAR szFreeBytes[28];
    unsigned int cchTotalBytes;
    unsigned int cchFreeBytes;
    cchTotalBytes = UI64ToStr(__emulu(dwBytesPerCluster,
                                      dwTotalNumberOfClusters),
                              _T(' '), szTotalBytes);
    cchFreeBytes = UI64ToStr(__emulu(dwBytesPerCluster,
                                     dwNumberOfFreeClusters),
                             _T(' '), szFreeBytes);
    if (cchFreeBytes < cchTotalBytes)
    {
      unsigned int cch = cchTotalBytes - cchFreeBytes;
      memmove(&szFreeBytes[cch], &szFreeBytes[0],
              (cchFreeBytes + 1) * sizeof(TCHAR));
#ifdef _UNICODE
      wmemset(&szFreeBytes[0], L' ', cch);
#else
      memset(&szFreeBytes[0], ' ', cch);
#endif  // _UNICODE
    }
    LogFmt(_T(" Bytes per Sector: %u\r\n")
           _T("Bytes per Cluster: %u\r\n")
           _T("      Total Bytes: %s\r\n")
           _T("       Free Bytes: %s\r\n"),
           dwBytesPerSector, dwBytesPerCluster, szTotalBytes, szFreeBytes);
  }
}
/***************************************************************************/
/* LogFixedDrivesInfo - Запись в файл отчета информации о жестких дисках   */
/***************************************************************************/
void LogFixedDrivesInfo()
{
  // Получение битовой карты доступных логических устройств
  DWORD dwLogicalDrives = ::GetLogicalDrives();
  if (dwLogicalDrives == 0)
    return;

  LogSectionTitle(_T("Logical Drives"));

  TCHAR szRootPath[4];
  szRootPath[1] = _T(':');
  szRootPath[2] = _T('\\');
  szRootPath[3] = _T('\0');
  // Отключение сообщений об ошибках
  unsigned int nOldErrorMode = ::SetErrorMode(SEM_FAILCRITICALERRORS);
  for (int i = 0; i < 26; i++)
  {
    if (!(dwLogicalDrives & (1 << i)))
      continue;
    szRootPath[0] = _T('A') + i;
    // Проверка типа диска
    if (::GetDriveType(szRootPath) == DRIVE_FIXED)
    {
      LogFmtTitle(_T("Drive %c"), szRootPath[0]);
      // Запись в файл отчета информации о жестком диске
      LogFixedDriveInfo(szRootPath);
      LogNewLine();
    }
  }
  ::SetErrorMode(nOldErrorMode);
  LogNewLine();
}
//---------------------------------------------------------------------------
