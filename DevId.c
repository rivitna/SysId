/***************************************************************************/
/* DevId.C - ������������� ��������� IDE/ATAPI/SCSI                        */
/* ������ 1.03 (������� 2012 �.)                                           */
/*                                                                         */
/* OS: Win32                                                               */
/*                                                                         */
/* Copyright (c) 2002-2012 rivitna                                         */
/***************************************************************************/
//---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <tchar.h>

#ifdef _USE_STDLIB
#include <memory.h>
#else
#include "StdUtils.h"
#endif  // _USE_STDLIB

#include "Ring0.h"
#include "DevId.h"
//---------------------------------------------------------------------------
#ifndef offsetof
#define offsetof(a, b)  ((size_t)&(((a *)0)->b))
#endif  // offsetof
//---------------------------------------------------------------------------
// ����������� ������� ���������� ATAPI
void DetectATAPIDevice();
// ������������� ���������� ATA/ATAPI
void IdentifyDevice();
// ��������� ���������� �� ���������� ATA/ATAPI
void __fastcall CorrectATADeviceInfo(
  PATA_DEVICE_INFO pDevInfo
  );
//---------------------------------------------------------------------------
/***************************************************************************/
/*                                                                         */
/*                         ������������� ���������                         */
/*                                                                         */
/***************************************************************************/

//
// ��������� � ����������� �� ����� "WinIoCtl.h"
//

#ifndef IOCTL_DISK_GET_DRIVE_GEOMETRY
#define IOCTL_DISK_GET_DRIVE_GEOMETRY    0x00070000
#endif  // IOCTL_DISK_GET_DRIVE_GEOMETRY

#ifndef IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER
#define IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER    0x002D0C10
#endif  // IOCTL_STORAGE_GET_MEDIA_SERIAL_NUMBER

typedef struct _MEDIA_SERIAL_NUMBER_DATA
{
  ULONG  SerialNumberLength;
  ULONG  Result;
  ULONG  Reserved[2];
  UCHAR  SerialNumberData[1];
} MEDIA_SERIAL_NUMBER_DATA, *PMEDIA_SERIAL_NUMBER_DATA;

#define MAX_MEDIA_SERIAL_NUMBER_LENGTH  256

/***************************************************************************/
/* NtGetPhysicalDriveHandle - ��������� ����������� ����������� ���������� */
/*                            ��: Windows NT � ����                        */
/***************************************************************************/
HANDLE NtGetPhysicalDriveHandle(
  BYTE bDevNum
  )
{
  const TCHAR szDeviceName[] = _T("\\\\.\\PhysicalDrive0");
  TCHAR szDevName[sizeof(szDeviceName)];

  if (bDevNum > MAX_DEVICE_NUM)
    return INVALID_HANDLE_VALUE;
  memcpy(szDevName, szDeviceName, sizeof(szDeviceName));
  szDevName[17] = '0' | bDevNum;
  return CreateFile(szDevName,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);
}
/***************************************************************************/
/* NtGetDiskGeometry - ��������� ���������� � ��������� ����������� �����  */
/*                     ��: Windows 2000 � ����                             */
/***************************************************************************/
BOOL NtGetDiskGeometry(
  HANDLE hDevice,
  PDISK_GEOMETRY pDiskGeometry
  )
{
  DWORD dwBytesReturned;
  return DeviceIoControl(hDevice,
                         IOCTL_DISK_GET_DRIVE_GEOMETRY,
                         NULL, 0,
                         (LPVOID)pDiskGeometry, sizeof(DISK_GEOMETRY),
                         &dwBytesReturned, NULL);
}
/***************************************************************************/
/* GetATADeviceSize - ��������� ������� ���������� ATA                     */
/***************************************************************************/
unsigned __int64 GetATADeviceSize(
  const ATA_DEVICE_INFO *pDevInfo
  )
{
#pragma pack(push, 1)
    union
    {
      unsigned __int64  qw;
      DWORD             dw[2];
      WORD              w[4];
    } nNumSecs;
#pragma pack(pop)

  // ����� 48-������ LBA?
  if (pDevInfo->wCommandSet2 & 0x400)
  {
    nNumSecs.dw[0] = pDevInfo->dwMaxLBA48Address[0];
    nNumSecs.dw[1] = pDevInfo->dwMaxLBA48Address[1];
#ifdef _USE_STDLIB
    return nNumSecs.qw * 512;
#else
    return Mul64(nNumSecs.qw, 512);
#endif  // _USE_STDLIB
  }
  // ����� 28-������ LBA?
  if (pDevInfo->dwTotalAddrSecs != 0)
    return ((unsigned __int64)pDevInfo->dwTotalAddrSecs * 512);
  return (pDevInfo->wCyls * pDevInfo->wHeads * pDevInfo->wSecsPerTrack * 512);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/***************************************************************************/
/*                                                                         */
/*                ������������� ��������� ATA/ATAPI (SMART)                */
/*                                                                         */
/***************************************************************************/

#ifndef SMART_GET_VERSION
#define SMART_GET_VERSION           0x00074080
#endif  // SMART_GET_VERSION
#ifndef SMART_SEND_DRIVE_COMMAND
#define SMART_SEND_DRIVE_COMMAND    0x0007C084
#endif  // SMART_SEND_DRIVE_COMMAND
#ifndef SMART_RCV_DRIVE_DATA
#define SMART_RCV_DRIVE_DATA        0x0007C088
#endif  // SMART_RCV_DRIVE_DATA

#pragma pack(push, 1)

typedef struct _GETVERSIONINPARAMS
{
  BYTE   bVersion;                      // Binary driver version
  BYTE   bRevision;                     // Binary driver revision
  BYTE   bReserved;                     // Not used
  BYTE   bIDEDeviceMap;                 // Bit map of IDE devices
  DWORD  fCapabilities;                 // Bit mask of driver capabilities
  DWORD  dwReserved[4];                 // For future use
} GETVERSIONINPARAMS, *PGETVERSIONINPARAMS, *LPGETVERSIONINPARAMS;

//
// Bits returned in the fCapabilities member of GETVERSIONINPARAMS
//

#define CAP_ATA_ID_CMD          1       // ATA ID command supported
#define CAP_ATAPI_ID_CMD        2       // ATAPI ID command supported
#define CAP_SMART_CMD           4       // SMART commannds supported

typedef struct _IDEREGS
{
  BYTE  bFeaturesReg;                   // Used for specifying SMART "commands"
  BYTE  bSectorCountReg;                // IDE sector count register
  BYTE  bSectorNumberReg;               // IDE sector number register
  BYTE  bCylLowReg;                     // IDE low order cylinder value
  BYTE  bCylHighReg;                    // IDE high order cylinder value
  BYTE  bDriveHeadReg;                  // IDE drive/head register
  BYTE  bCommandReg;                    // Actual IDE command
  BYTE  bReserved;                      // Reserved for future use. Must be zero.
} IDEREGS, *PIDEREGS, *LPIDEREGS;

//
// Valid values for the bCommandReg member of IDEREGS
//

#define ATAPI_ID_CMD    0xA1            // Returns ID sector for ATAPI
#define ID_CMD          0xEC            // Returns ID sector for ATA
#define SMART_CMD       0xB0            // Performs SMART cmd
                                        // Requires valid bFeaturesReg,
                                        // bCylLowReg, and bCylHighReg

//
// Cylinder register defines for SMART command
//

#define SMART_CYL_LOW   0x4F
#define SMART_CYL_HI    0xC2

typedef struct _SENDCMDINPARAMS
{
  DWORD    cBufferSize;                 // Buffer size in bytes
  IDEREGS  irDriveRegs;                 // Structure with drive register values
  BYTE     bDriveNumber;                // Physical drive number to send
                                        // command to (0,1,2,3)
  BYTE     bReserved[3];                // Reserved for future expansion
  DWORD    dwReserved[4];               // For future use
  BYTE     bBuffer[1];                  // Input buffer
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;

typedef struct _DRIVERSTATUS
{
  BYTE   bDriverError;                  // Error code from driver,
                                        // or 0 if no error
  BYTE   bIDEError;                     // Contents of IDE Error register
                                        // Only valid when bDriverError
                                        // is SMART_IDE_ERROR
  BYTE   bReserved[2];                  // Reserved for future expansion
  DWORD  dwReserved[2];                 // Reserved for future expansion
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;

//
// bDriverError values
//

#define SMART_NO_ERROR              0   // No error
#define SMART_IDE_ERROR             1   // Error from IDE controller
#define SMART_INVALID_FLAG          2   // Invalid command flag
#define SMART_INVALID_COMMAND       3   // Invalid command byte
#define SMART_INVALID_BUFFER        4   // Bad buffer (null, invalid addr..)
#define SMART_INVALID_DRIVE         5   // Drive number not valid
#define SMART_INVALID_IOCTL         6   // Invalid IOCTL
#define SMART_ERROR_NO_MEM          7   // Could not lock user's buffer
#define SMART_INVALID_REGISTER      8   // Some IDE Register not valid
#define SMART_NOT_SUPPORTED         9   // Invalid cmd flag set
#define SMART_NO_IDE_DEVICE         10  // Cmd issued to device not present
                                        // although drive number is valid

typedef struct _SENDCMDOUTPARAMS
{
  DWORD         cBufferSize;            // Size of bBuffer in bytes
  DRIVERSTATUS  DriverStatus;           // Driver status structure
  BYTE          bBuffer[1];             // Buffer of arbitrary length in which
                                        // to store the data read from the drive
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;

#pragma pack(pop)

/***************************************************************************/
/* SMART_IdentifyDevice - ������������� ���������� ATA/ATAPI (SMART)       */
/*                        ��: Windows NT � ����                            */
/***************************************************************************/
BOOL SMART_IdentifyDevice(
  HANDLE hDevice,
  BYTE bDevNum,
  PATA_DEVICE_INFO pDevInfo
  )
{
  GETVERSIONINPARAMS gvip;
  SENDCMDINPARAMS scip;
  BYTE scop[sizeof(SENDCMDOUTPARAMS) + DEVICE_INFO_BUFFER_SIZE - 1];
  DWORD dwBytesReturned;

  bDevNum &= MAX_DEVICE_NUM;

  // ��������� ���������� � ������ ��������
  ZeroMemory(&gvip, sizeof(gvip));
  if (!DeviceIoControl(hDevice,
                       SMART_GET_VERSION,
                       NULL, 0,
                       (LPVOID)&gvip, sizeof(gvip),
                       &dwBytesReturned, NULL))
    return FALSE;

  // ������������� ���������� ATA/ATAPI
  ZeroMemory(&scip, sizeof(scip));
  ZeroMemory(&scop, sizeof(scop));
  scip.irDriveRegs.bSectorCountReg = 1;
  scip.irDriveRegs.bSectorNumberReg = 1;
  scip.irDriveRegs.bDriveHeadReg = 0xA0 | ((bDevNum & 1) << 4);
  scip.irDriveRegs.bCommandReg = ((gvip.bIDEDeviceMap >> bDevNum) & 0x10)
                                   ? ATAPI_ID_CMD : ID_CMD;
  scip.bDriveNumber = bDevNum;
  scip.cBufferSize = DEVICE_INFO_BUFFER_SIZE;
  if (!DeviceIoControl(hDevice,
                       SMART_RCV_DRIVE_DATA,
                       (LPVOID)&scip, sizeof(scip) - 1,
                       (LPVOID)&scop, sizeof(scop),
                       &dwBytesReturned, NULL))
    return FALSE;
  if (((PSENDCMDOUTPARAMS)&scop)->DriverStatus.bDriverError != SMART_NO_ERROR)
    return SetLastError(ERROR_INVALID_DATA), FALSE;
  // ��������� ���������� �� ���������� ATA/ATAPI
  CorrectATADeviceInfo((PATA_DEVICE_INFO)((PSENDCMDOUTPARAMS)&scop)->bBuffer);
  // ����������� � �����
  memcpy(pDevInfo, ((PSENDCMDOUTPARAMS)&scop)->bBuffer,
         DEVICE_INFO_BUFFER_SIZE);
  return TRUE;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/***************************************************************************/
/*                                                                         */
/*                      ������������� ��������� SCSI                       */
/*                                                                         */
/***************************************************************************/

//
// ��������� � ����������� �� ������ DDK "ntddscsi.h", "scsi.h"
//

#ifndef IOCTL_SCSI_PASS_THROUGH
#define IOCTL_SCSI_PASS_THROUGH    0x0004D004
#endif  // IOCTL_SCSI_PASS_THROUGH

//
// Define the SCSI pass through structure
//

typedef struct _SCSI_PASS_THROUGH
{
  USHORT     Length;
  UCHAR      ScsiStatus;
  UCHAR      PathId;
  UCHAR      TargetId;
  UCHAR      Lun;
  UCHAR      CdbLength;
  UCHAR      SenseInfoLength;
  UCHAR      DataIn;
  ULONG      DataTransferLength;
  ULONG      TimeOutValue;
  ULONG_PTR  DataBufferOffset;
  ULONG      SenseInfoOffset;
  UCHAR      Cdb[16];
} SCSI_PASS_THROUGH, *PSCSI_PASS_THROUGH;

//
// Command Descriptor Block constants
//

#define CDB6GENERIC_LENGTH     6
#define CDB10GENERIC_LENGTH    10
#define CDB12GENERIC_LENGTH    12

//
// Define values for pass-through DataIn field
//

#define SCSI_IOCTL_DATA_OUT            0
#define SCSI_IOCTL_DATA_IN             1
#define SCSI_IOCTL_DATA_UNSPECIFIED    2

// Cdb
#define SCSIOP_INQUIRY    0x12  // Operation Code

//
// Enable Vital Product Data Flag (EVPD)
// used with INQUIRY command
//

#define CDB_INQUIRY_EVPD    0x01

#define SPT_SENSE_BUFFER_SIZE  32
#define SPT_DATA_BUFFER_SIZE   192

typedef struct _SCSI_PASS_THROUGH_WITH_BUFFERS
{
  SCSI_PASS_THROUGH  spt;
  BYTE               SenseBuf[SPT_SENSE_BUFFER_SIZE];
  BYTE               DataBuf[SPT_DATA_BUFFER_SIZE];
} SCSI_PASS_THROUGH_WITH_BUFFERS, *PSCSI_PASS_THROUGH_WITH_BUFFERS;

/***************************************************************************/
/* NtGetSCSIDeviceSerialNumber - ��������� ��������� ������ ���������� SCSI*/
/*                               ��: Windows NT � ����                     */
/***************************************************************************/
BOOL NtGetSCSIDeviceSerialNumber(
  HANDLE hDevice,
  LPTSTR pBuffer,
  UINT *pnSize
  )
{
  BYTE sptwb[sizeof(SCSI_PASS_THROUGH_WITH_BUFFERS) +
             sizeof(SCSI_PASS_THROUGH)];
  PSCSI_PASS_THROUGH pspt;
  DWORD dwBytesReturned;
  UINT cch;

  // ��������� ��������� ������ ���������� SCSI
  ZeroMemory(&sptwb, sizeof(sptwb));
  pspt = &((PSCSI_PASS_THROUGH_WITH_BUFFERS)&sptwb)->spt;
  pspt->Length = sizeof(SCSI_PASS_THROUGH);
  pspt->CdbLength = CDB6GENERIC_LENGTH;
  pspt->SenseInfoLength = SPT_SENSE_BUFFER_SIZE;
  pspt->DataIn = SCSI_IOCTL_DATA_IN;
  pspt->DataTransferLength = SPT_DATA_BUFFER_SIZE;
  pspt->TimeOutValue = 2L;
  pspt->DataBufferOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, DataBuf);
  pspt->SenseInfoOffset = offsetof(SCSI_PASS_THROUGH_WITH_BUFFERS, SenseBuf);
  pspt->Cdb[0] = SCSIOP_INQUIRY;        // Operation Code
  pspt->Cdb[1] = CDB_INQUIRY_EVPD;      // Flags: Enable Vital product data
  pspt->Cdb[2] = 0x80;                  // Page Code: Unit serial number
  pspt->Cdb[4] = SPT_DATA_BUFFER_SIZE;  // Allocation Length
  if (!DeviceIoControl(hDevice,
                       IOCTL_SCSI_PASS_THROUGH,
                       &sptwb, sizeof(SCSI_PASS_THROUGH),
                       &sptwb,
                       pspt->DataBufferOffset + pspt->DataTransferLength,
                       &dwBytesReturned, NULL))
    return FALSE;
  if (((PSCSI_PASS_THROUGH_WITH_BUFFERS)&sptwb)->DataBuf[1] != 0x80)
    return SetLastError(ERROR_INVALID_DATA), FALSE;
  cch = ((PSCSI_PASS_THROUGH_WITH_BUFFERS)&sptwb)->DataBuf[3];
  if ((pBuffer == NULL) || (cch >= *pnSize))
  {
    *pnSize = cch + 1;
    return SetLastError(ERROR_INSUFFICIENT_BUFFER), FALSE;
  }
#ifdef _UNICODE
  cch =
    MultiByteToWideChar(CP_ACP, 0,
                        ((PSCSI_PASS_THROUGH_WITH_BUFFERS)&sptwb)->DataBuf + 4,
                        cch,
                        pBuffer, *pnSize);
#else
  memcpy(pBuffer, ((PSCSI_PASS_THROUGH_WITH_BUFFERS)&sptwb)->DataBuf + 4,
         cch);
#endif  /* _UNICODE */
  pBuffer[cch] = _T('\0');
  *pnSize = cch;
  return TRUE;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/***************************************************************************/
/*                                                                         */
/*             ������������� ��������� ATA/ATAPI (Windows 9x)              */
/*                                                                         */
/***************************************************************************/

// ������, ������������ � ��������� Ring0_IdentifyDevice
#pragma pack(push, 1)
typedef struct _RING0_ID_DEV_DATA_IN
{
  WORD  wBasePort;
  BYTE  bDevNum;
  void *pBuffer;
} RING0_ID_DEV_DATA_IN, *PRING0_ID_DEV_DATA_IN;
#pragma pack(pop)

/***************************************************************************/
/* Ring0_IdentifyDevice - ������������� ���������� ATA/ATAPI (Ring 0)      */
/***************************************************************************/
/* In:   ECX(EAX) -> ������ ��� ��������� ���������� (RING0_ID_DEV_DATA_IN)*/
/* Out:  EAX = ���� ���������/���������� ����������                        */
/***************************************************************************/
__declspec(naked) void Ring0_IdentifyDevice()
{
  __asm
  {
    push  edi

#ifdef __BORLANDC__
    mov   edi,[eax+3]        // EDI -> �����
    mov   dx,[eax]           // DX = ����� �������� �����
    mov   al,[eax+2]         // AL = ����� ����������
#else
    mov   dx,[ecx]           // DX = ����� �������� �����
    mov   al,[ecx+2]         // AL = ����� ����������
    mov   edi,[ecx+3]        // EDI -> �����
#endif  // __BORLANDC__

    push  eax
    push  edx
    call  DetectATAPIDevice  // ���������� ATAPI?
    pop   edx
    pop   eax
    mov   ah,0ECh            // AH=0ECh (������� ������������� ���������� ATA)
    jc    IdentifyDev
    mov   ah,0A1h            // AH=0A1h (������� ������������� ���������� ATAPI)
IdentifyDev:
    call  IdentifyDevice     // ������������� ���������� ATA/ATAPI
    setnc al
    movzx eax,al

    pop   edi
    ret
  }
}
/***************************************************************************/
/* Win9x_IdentifyDevice - ������������� ���������� ATA/ATAPI (Windows 9x)  */
/***************************************************************************/
BOOL Win9x_IdentifyDevice(
  WORD wBasePort,
  BYTE bDevNum,
  PATA_DEVICE_INFO pDevInfo
  )
{
  BOOL bSuccess = FALSE;
  RING0_ID_DEV_DATA_IN r0idd;
  // ����� ��������� ������������� ���������� ATA/ATAPI � Ring 0
  r0idd.wBasePort = wBasePort;
  r0idd.bDevNum = bDevNum;
  r0idd.pBuffer = pDevInfo;
  CallRing0((PRING0FUNC)Ring0_IdentifyDevice, (DWORD)&r0idd,
            (LPDWORD)&bSuccess);
  if (!bSuccess)
    return FALSE;
  // ��������� ���������� �� ���������� ATA/ATAPI
  CorrectATADeviceInfo(pDevInfo);
  return TRUE;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/***************************************************************************/
/*                                                                         */
/*                    ������������� ��������� ATA/ATAPI                    */
/*                                                                         */
/***************************************************************************/

/***************************************************************************/
/* DevWait - ��������, ���� ���������� ������                              */
/***************************************************************************/
/* In:   DX = ������� �������/���������                                    */
/* Out:  CF = ���� ���������/���������� ����������                         */
/* Regs: AL, ECX, Flags                                                    */
/***************************************************************************/
__declspec(naked) void DevWait()
{
  __asm
  {
    mov   ecx,140000h
WaitLoop:
    in    al,dx              // AL = ���� ��������� ����������
    test  al,80h             // ���������� ������?
    jz    Done
    loop  WaitLoop

    stc

Done:
    ret
  }
}
/***************************************************************************/
/* DetectATAPIDevice - ����������� ������� ���������� ATAPI                */
/***************************************************************************/
/* In:   DX = ������� ����                                                 */
/*       AL = ����� ����������                                             */
/* Out:  CF = ���� ���������/���������� ����������                         */
/* Regs: AX, ECX, DX, Flags                                                */
/***************************************************************************/
__declspec(naked) void DetectATAPIDevice()
{
  __asm
  {
    and   al,1
    shl   al,4
    or    al,0A0h
    add   dx,6               // DX = ������� ������ ����������/�������
    out   dx,al              // ����� ����������

    inc   dx                 // DX = ������� �������/���������
    in    al,dx              // AL = ���� ��������� ����������
    cmp   al,0FFh
    je    NoDevice

//    mov   al,8               // AL=8 (������� ������ ������)
//    out   dx,al

//    call  DevWait
//    jc    NoDevice

    sub   dx,3               // DX = ������� �������� (������� ����)
    xor   al,al
    out   dx,al
    inc   dx                 // DX = ������� �������� (������� ����)
    out   dx,al

    inc   dx
    inc   dx                 // DX = ������� �������/���������
    mov   al,0ECh            // AL=0ECh (������� ������������� ����������)
    out   dx,al

    call  DevWait
    jc    NoDevice

    sub   dx,3               // DX = ������� �������� (������� ����)
    in    al,dx
    mov   ah,al
    inc   dx                 // DX = ������� �������� (������� ����)
    in    al,dx
    cmp   ax,14EBh
    je    Done

NoDevice:
    stc

Done:
    ret
  }
}
/***************************************************************************/
/* IdentifyDevice - ������������� ���������� ATA/ATAPI                     */
/***************************************************************************/
/* In:   DX = ������� ����                                                 */
/*       AL = ����� ����������                                             */
/*       AH = ������� ������������� ���������� (ATA - 0CEh, ATAPI - 0A1h)  */
/*       EDI -> ����� ��� ���������� �� ����������                         */
/* Out:  CF = ���� ���������/���������� ����������                         */
/* Regs: AL, ECX, DX, Flags                                                */
/***************************************************************************/
__declspec(naked) void IdentifyDevice()
{
  __asm
  {
    and   al,1
    shl   al,4
    or    al,0A0h
    add   dx,6               // DX = ������� ������ ����������/�������
    out   dx,al              // ����� ����������

    inc   dx                 // DX = ������� �������/���������
    in    al,dx              // AL = ���� ��������� ����������
    cmp   al,0FFh
    je    Error

    call  DevWait
    jc    Error

    mov   al,ah              // AL = ������� ������������� ����������
    out   dx,al              // ������� ������������� ����������

    call  DevWait
    jc    Error

    mov   ecx,100h
WaitLoop:
    in    al,dx              // AL = ���� ��������� ����������
    test  al,8               // ���������� ��� DRQ?
    loopz WaitLoop
    jz    Error

    sub   dx,7               // DX = ������� ������
    push  edi
    mov   ecx,100h
    cld
    cli
    rep   insw               // ������ ������
    sti
    pop   edi

    add   dx,7               // DX = ������� �������/���������
    in    al,dx              // AL = ���� ��������� ����������
    and   al,71h
    cmp   al,50h
    je    Done

Error:
    stc

Done:
    ret
  }
}
/***************************************************************************/
/* CorrectATADeviceInfo - ��������� ���������� �� ���������� ATA/ATAPI     */
/***************************************************************************/
/* In:   ECX -> ����� ��� ���������� �� ����������                         */
/* Out:  ���                                                               */
/* Regs: AX, ECX, Flags                                                    */
/***************************************************************************/
__declspec(naked) void __fastcall CorrectATADeviceInfo(
  PATA_DEVICE_INFO pDevInfo
  )
{
  __asm
  {
    push  edi

    cld

    lea   edi,[ecx+14h]      // EDI -> sSerialNumber

    mov   ecx,20 SHR 1
SerNumLoop:
    mov   ax,[edi]
    xchg  al,ah
    stosw
    loop  SerNumLoop

    add   edi,6              // EDI -> sFirmwareRev
    mov   cl,(8+40) SHR 1
ModelNumLoop:
    mov   ax,[edi]
    xchg  al,ah
    stosw
    loop  ModelNumLoop

    pop   edi

    ret
  }
}
//---------------------------------------------------------------------------
