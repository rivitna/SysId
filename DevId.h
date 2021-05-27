/***************************************************************************/
/* DevId.C - Идентификация устройств IDE/ATAPI/SCSI                        */
/* Версия 1.03 (Октябрь 2012 г.)                                           */
/*                                                                         */
/* OS: Win32                                                               */
/*                                                                         */
/* Copyright (c) 2002-2012 rivitna                                         */
/***************************************************************************/
//---------------------------------------------------------------------------
#ifndef __DEVID_H__
#define __DEVID_H__
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
//---------------------------------------------------------------------------
/***************************************************************************/
/*                                                                         */
/*                         Идентификация устройств                         */
/*                                                                         */
/***************************************************************************/

//
// Структуры и определения из файла "WinIoCtl.h"
//

typedef struct _DISK_GEOMETRY
{
  LARGE_INTEGER  Cylinders;
  ULONG          MediaType;
  ULONG          TracksPerCylinder;
  ULONG          SectorsPerTrack;
  ULONG          BytesPerSector;
} DISK_GEOMETRY, *PDISK_GEOMETRY;

// MediaType
#define RemovableMedia    0x0B  // Съемный носитель
#define FixedMedia        0x0C  // Жесткий диск

#pragma pack(push, 1)

// Блок информации, передаваемый по ATA-команде идентификации устройства
// IDENTIFY DEVICE (ECh)
typedef struct _ATA_DEVICE_INFO
{
  WORD   wGenConfig;            // 0000: General configuration
                                //          15   0 = ATA device
                                //         14-8  Retired
                                //           7   1 = removable media device
                                //           6   1 = not removable controller
                                //                    and/or device
                                //          5-3  Retired
                                //           2   Response incomplete
                                //           1   Retired
                                //           0   Reserved
  WORD   wCyls;                 // 0002: Number of logical cylinders
  WORD   wSpecConfig;           // 0004: Specific configuration
  WORD   wHeads;                // 0006: Number of logical heads
  WORD   wBytesPerTrack;        // 0008: Retired
  WORD   wBytesPerSector;       // 000A: Retired
  WORD   wSecsPerTrack;         // 000C: Number of logical sectors per
                                //       logical track
  WORD   wCFAReserved1[2];      // 000E: Reserved for assignment by
                                //       the CompactFlash Association
  WORD   wNotUsed1;             // 0012: Retired
  CHAR   sSerialNumber[20];     // 0014: Serial number
  WORD   wBufferType;           // 0028: Retired
  WORD   wBufferSize;           // 002A: Retired
  WORD   wECCBytes;             // 002C: Obsolete
  CHAR   sFirmwareRev[8];       // 002E: Firmware revision
  CHAR   sModelNumber[40];      // 0036: Model number
  WORD   wRWMultiple;           // 005E: READ/WRITE MULTIPLE support
  WORD   wDoubleWordIO;         // 0060: Reserved
  WORD   wCapabilities1;        // 0062: Capabilities
  WORD   wCapabilities2;        // 0064: Capabilities
  WORD   wPIOTimingMode;        // 0066: Obsolete
  WORD   wDMATimingMode;        // 0068: Obsolete
  WORD   wFieldValidity;        // 006A: Field validity
  WORD   wCurrCyls;             // 006C: Number of current logical cylinders
  WORD   wCurrHeads;            // 006E: Number of current logical heads
  WORD   wCurrSecsPerTrack;     // 0070: Number of current logical sectors
                                //       per track
  DWORD  dwCurrCapacity;        // 0072: Current capacity in sectors
  WORD   wMultiSectorSetting;   // 0076: Multiple sector setting
  DWORD  dwTotalAddrSecs;       // 0078: Total number of user addressable
                                //       sectors (28-bit LBA)
  WORD   wSingleWordDMA;        // 007C: Obsolete
  WORD   wMultiWordDMA;         // 007E: Multiword DMA transfer
  WORD   wAdvancedPIOModes;     // 0080: PIO transfer modes supported
  WORD   wMinDMACycle;          // 0082: Minimum Multiword DMA transfer cycle
                                //       time per word
  WORD   wRecDMACycle;          // 0084: Device recommended Multiword DMA
                                //       cycle time
  WORD   wMinPIOCycle;          // 0086: Minimum PIO transfer cycle time
                                //       without flow control
  WORD   wMinPIOCycleIORDY;     // 0088: Minimum PIO transfer cycle time
                                //       with IORDY
  WORD   wReserved1[6];         // 008A: Reserved
  WORD   wQueueDepth;           // 0096: Queue depth
  WORD   wReserved2[4];         // 0098: Reserved
  WORD   wMajorVerNum;          // 00A0: Major version number
  WORD   wMinorVerNum;          // 00A2: Minor version number
  WORD   wCommandSet1;          // 00A4: Command set supported
  WORD   wCommandSet2;          // 00A6: Command set supported
  WORD   wCommandSetExt;        // 00A8: Command set/feature supported extension
  WORD   wCommandSetEnabled1;   // 00AA: Command set/feature enabled
  WORD   wCommandSetEnabled2;   // 00AC: Command set/feature enabled
  WORD   wCommandSetDefault;    // 00AE: Command set/feature default
  WORD   wUDMAModes;            // 00B0: Ultra DMA modes
  WORD   wReqTimeSecErase;      // 00B2: Time required for security erase
                                //       unit completion
  WORD   wReqTimeEnhSecErase;   // 00B4: Time required for Enhanced security
                                //       erase completion
  WORD   wAdvPowerMgmtLevel;    // 00B6: Advanced power management level value
  WORD   wMasterPwdRevCode;     // 00B8: Master Password Revision Code
  WORD   wHWCfgTestResults;     // 00BA: Hardware configuration test results
  WORD   wReserved3[6];         // 00BC: Reserved
  DWORD  dwMaxLBA48Address[2];  // 00C8: Maximum user LBA address for
                                //       48-bit Address feature set
  WORD   wReserved4[23];        // 00D0: Reserved
  WORD   wRemMediaStatusNotif;  // 00FE: Removable Media Status Notification
                                //       feature set support
  WORD   wSecurityStatus;       // 0100: Security status
  WORD   wVendorSpec1[31];      // 0102: Vendor specific
  WORD   wCFAPowerMode;         // 0140: CFA power mode
  WORD   wCFAReserved2[15];     // 0142: Reserved for assignment by
                                //       the CompactFlash Association
  WORD   wReserved5[79];        // 0160: Reserved
  WORD   wIntegrity;            // 01FE: Integrity word
} ATA_DEVICE_INFO, *PATA_DEVICE_INFO;

// Блок информации, передаваемый по ATA-команде идентификации пакетного
// устройства IDENTIFY PACKET DEVICE (A1h)
typedef struct _ATAPI_DEVICE_INFO
{
  WORD   wGenConfig;            // 0000: General configuration
                                //        15-14  10 = ATAPI device
                                //               11 = Reserved
                                //          13   Reserved
                                //         12-8  Field indicates command
                                //               packet set used by device
                                //               05 = CD-ROM
                                //           7   1 = removable media device
                                //          6-5  00 = Device shall set DRQ
                                //                    to one within 3 ms of
                                //                    receiving PACKET command
                                //               01 = Obsolete
                                //               10 = Device shall set DRQ
                                //                    to one within 50 microseconds
                                //                    of receiving PACKET command
                                //               11 = Reserved
                                //          4-3  Reserved
                                //           2   Incomplete response
                                //          1-0  00 = 12 byte command packet
                                //               01 = 16 byte command packet
                                //               1x = Reserved
  WORD   wReserved1;            // 0002: Reserved
  WORD   wSpecConfig;           // 0004: Specific configuration
  WORD   wReserved2[7];         // 0006: Reserved
  CHAR   sSerialNumber[20];     // 0014: Serial number
  WORD   wReserved3[3];         // 0028: Reserved
  CHAR   sFirmwareRev[8];       // 002E: Firmware revision
  CHAR   sModelNumber[40];      // 0036: Model number
  WORD   wReserved4[2];         // 005E: Reserved
  WORD   wCapabilities;         // 0062: Capabilities
  WORD   wReserved5;            // 0064: Reserved
  WORD   wNotUsed1[2];          // 0066: Obsolete
  WORD   wFieldValidity;        // 006A: Field validity
  WORD   wReserved6[9];         // 006C: Reserved
  WORD   wMultiWordDMA;         // 007E: Multiword DMA transfer
  WORD   wAdvancedPIO;          // 0080: PIO transfer modes supported
  WORD   wMinDMACycle;          // 0082: Minimum Multiword DMA transfer
                                //       cycle time per word
  WORD   wRecDMACycle;          // 0084: Device recommended Multiword DMA
                                //       cycle time
  WORD   wMinPIOCycle;          // 0086: Minimum PIO transfer cycle time
                                //       without flow control
  WORD   wMinPIOCyclewIORDY;    // 0088: Minimum PIO transfer cycle time
                                //       with IORDY
  WORD   wReserved7[2];         // 008A: Reserved
  WORD   wPACKETBusRlseTime;    // 008E: PACKET to bus release time
  WORD   wSERVICEBusRlseTime;   // 0090: SERVICE to bus release time
  WORD   wReserved8[2];         // 0092: Reserved
  WORD   wQueueDepth;           // 0096: Queue depth
  WORD   wReserved9[4];         // 0098: Reserved
  WORD   wMajorVerNum;          // 00A0: Major version number
  WORD   wMinorVerNum;          // 00A2: Minor version number
  WORD   wCommandSet1;          // 00A4: Command set supported
  WORD   wCommandSet2;          // 00A6: Command set supported
  WORD   wCommandSetExt;        // 00A8: Command set/feature supported extension
  WORD   wCommandSetEnabled1;   // 00AA: Command set/feature enabled
  WORD   wCommandSetEnabled2;   // 00AC: Command set/feature enabled
  WORD   wCommandSetDefault;    // 00AE: Command set/feature default
  WORD   wUDMAModes;            // 00B0: Ultra DMA modes
  WORD   wReqTimeSecErase;      // 00B2: Time required for security erase
                                //       unit completion
  WORD   wReqTimeEnhSecErase;   // 00B4: Time required for Enhanced security
                                //       erase completion
  WORD   wReserved10[2];        // 00B6: Reserved
  WORD   wHWResetResults;       // 00BA: Hardware reset results
  WORD   wReserved11[32];       // 00BC: Reserved
  WORD   wATAPIByteCount0Bhv;   // 00FC: ATAPI byte count = 0 behavior
  WORD   wRemMediaStatNotif;    // 00FE: Removable Media Status Notification
                                //       feature set support
  WORD   wSecurityStatus;       // 0100: Security status
  WORD   wVendorSpec1[31];      // 0102: Vendor specific
  WORD   wCFAReserved2[16];     // 0140: Reserved for assignment by
                                //       the CompactFlash Association
  WORD   wReserved12[79];       // 0160: Reserved
  WORD   wIntegrity;            // 01FE: Integrity word
} ATAPI_DEVICE_INFO, *PATAPI_DEVICE_INFO;

#pragma pack(pop)

// Размер буфера для идентификации устройства ATA/ATAPI
#define DEVICE_INFO_BUFFER_SIZE  512
// Максимальный номер устройства
#define MAX_DEVICE_NUM  7

// Получение дескриптора физического устройства
// ОС: Windows NT и выше
HANDLE NtGetPhysicalDriveHandle(
  BYTE bDevNum
  );
// Получение информации о геометрии физического диска
// ОС: Windows 2000 и выше
BOOL NtGetDiskGeometry(
  HANDLE hDevice,
  PDISK_GEOMETRY pDiskGeometry
  );
// Получение размера устройства ATA
unsigned __int64 GetATADeviceSize(
  const ATA_DEVICE_INFO *pDevInfo
  );
// Идентификация устройства ATA/ATAPI (SMART)
// ОС: Windows NT и выше
BOOL SMART_IdentifyDevice(
  HANDLE hDevice,
  BYTE bDevNum,
  PATA_DEVICE_INFO pDevInfo
  );
// Получение серийного номера устройства SCSI
// ОС: Windows NT и выше
BOOL NtGetSCSIDeviceSerialNumber(
  HANDLE hDevice,
  LPTSTR pBuffer,
  UINT *pnSize
  );
// Идентификация устройства ATA/ATAPI (Windows 9x)
BOOL Win9x_IdentifyDevice(
  WORD wBasePort,
  BYTE bDevNum,
  PATA_DEVICE_INFO pDevInfo
  );
//---------------------------------------------------------------------------
#ifdef __cplusplus
}  // extern "C"
#endif // __cplusplus
//---------------------------------------------------------------------------
#endif  // __DEVID_H__
