//---------------------------------------------------------------------------
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <tchar.h>

#ifdef _USE_STDLIB
#include <malloc.h>
#else
#include "StdUtils.h"
#endif  // _USE_STDLIB

#include "AppUtils.h"
//---------------------------------------------------------------------------
// ���������� ����� ������
HANDLE g_hLogFile = NULL;
//---------------------------------------------------------------------------
/***************************************************************************/
/* OpenLog - �������� ����� ������                                         */
/***************************************************************************/
BOOL OpenLog(
  const TCHAR *pszFileName,
  BOOL bAppend
  )
{
  // �������� ����� ������
  CloseLog();
  // �������� ����� ������
  g_hLogFile = CreateFile(pszFileName,
                          bAppend ? (GENERIC_READ | GENERIC_WRITE)
                                  : GENERIC_WRITE,
                          FILE_SHARE_READ,
                          NULL,
                          bAppend ? OPEN_ALWAYS : CREATE_ALWAYS,
                          FILE_FLAG_SEQUENTIAL_SCAN,
                          NULL);
  if (g_hLogFile == INVALID_HANDLE_VALUE)
    return FALSE;
  if (bAppend)
  {
    // ����������� ��������� ����� � �����
    SetFilePointer(g_hLogFile, 0, NULL, FILE_END);
  }
  return TRUE;
}
/***************************************************************************/
/* CloseLog - �������� ����� ������                                        */
/***************************************************************************/
void CloseLog(void)
{
  if ((g_hLogFile != INVALID_HANDLE_VALUE) && (g_hLogFile != NULL))
  {
    // �������� ����������� ����� ������
    CloseHandle(g_hLogFile);
    g_hLogFile = INVALID_HANDLE_VALUE;
  }
}
/***************************************************************************/
/* Log - ������ � ���� ������                                              */
/***************************************************************************/
int Log(
  const TCHAR *pszMsg
  )
{
#ifdef _UNICODE
  char buf[1024];
  char *pchBuf = buf;
  int nBufSize = sizeof(buf);
  int nRet = 0;
  int cch;
  DWORD cb;

  if ((g_hLogFile == INVALID_HANDLE_VALUE) || (g_hLogFile == NULL))
    return 0;
  cch = lstrlen(pszMsg);
  if (cch == 0)
    return 0;
  cb = WideCharToMultiByte(CP_ACP, 0, pszMsg, cch, NULL, 0, NULL, NULL);
  if (cb > sizeof(buf))
  {
    pchBuf = (char *)malloc(cb);
    if (pchBuf == NULL)
      return 0;
    nBufSize = cb;
  }
  cb = WideCharToMultiByte(CP_ACP, 0, pszMsg, cch, pchBuf, nBufSize,
                           NULL, NULL);
  if (cb != 0)
  {
    if (WriteFile(g_hLogFile, pchBuf, cb, &cb, NULL))
      nRet = (int)cb;
  }
  if (pchBuf != buf) free(pchBuf);
  return nRet;
#else
  DWORD dwBytesWritten;
  if ((g_hLogFile != INVALID_HANDLE_VALUE) && (g_hLogFile != NULL) &&
      WriteFile(g_hLogFile, pszMsg, lstrlen(pszMsg), &dwBytesWritten, NULL))
    return (int)dwBytesWritten;
  return 0;
#endif  // _UNICODE
}
/***************************************************************************/
/* LogFmtV - ��������������� ������ � ���� ������                          */
/***************************************************************************/
int LogFmtV(
  const TCHAR *pszFmt,
  va_list arglist
  )
{
  TCHAR buf[1028];
  wvsprintf(buf, pszFmt, arglist);
  // ������ � ���� ������
  return Log(buf);
}
/***************************************************************************/
/* LogFmt - ��������������� ������ � ���� ������                           */
/***************************************************************************/
int LogFmt(
  const TCHAR *pszFmt,
  ...
  )
{
  int nRet;
  va_list arglist;
  va_start(arglist, pszFmt);
  // ��������������� ������ � ���� ������
  nRet = LogFmtV(pszFmt, arglist);
  va_end(arglist);
  return nRet;
}
/***************************************************************************/
/* LogNewLine - ���������� � ���� ������ ����� ������                      */
/***************************************************************************/
int LogNewLine(void)
{
  // ������ � ���� ������
  return Log(_T("\r\n"));
}
/***************************************************************************/
/* PrintOem - ����� ������ � ��������� OEM �� �����                        */
/***************************************************************************/
int PrintOem(
  const char *pszOemText
  )
{
  DWORD dwCharsWritten;
  WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),
            pszOemText, lstrlenA(pszOemText),
            &dwCharsWritten, NULL);
  return (int)dwCharsWritten;
}
/***************************************************************************/
/* Print - ����� ������ �� �����                                           */
/***************************************************************************/
int Print(
  const TCHAR *pszText
  )
{
#ifdef _UNICODE
  DWORD dwCharsWritten;
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), pszText, lstrlen(pszText),
               &dwCharsWritten, NULL);
  return (int)dwCharsWritten;
#else
  char buf[1024];
  char *pchBuf = buf;
  unsigned int cbText;
  int nRet = 0;

  cbText = lstrlen(pszText);
  if (cbText >= sizeof(buf))
  {
    pchBuf = (char *)malloc(cbText + 1);
    if (pchBuf == NULL)
      return 0;
  }
  if (CharToOem(pszText, pchBuf))
  {
    // ����� ������ � ��������� OEM �� �����
    nRet = PrintOem(pchBuf);
  }
  if (pchBuf != buf) free(pchBuf);
  return nRet;
#endif  // _UNICODE
}
/***************************************************************************/
/* PrintFmtV - ��������������� ����� ������ �� �����                       */
/***************************************************************************/
int PrintFmtV(
  const TCHAR *pszFmt,
  va_list arglist
  )
{
  TCHAR buf[1028];
  wvsprintf(buf, pszFmt, arglist);
  // ����� ������ �� �����
  return Print(buf);
}
/***************************************************************************/
/* PrintFmt - ��������������� ����� ������ �� �����                        */
/***************************************************************************/
int PrintFmt(
  const TCHAR *pszFmt,
  ...
  )
{
  int nRet;
  va_list arglist;
  va_start(arglist, pszFmt);
  // ��������������� ����� ������ �� �����
  nRet = PrintFmtV(pszFmt, arglist);
  va_end(arglist);
  return nRet;
}
/***************************************************************************/
/* PrintNewLine - ������� �� ����� ������                                  */
/***************************************************************************/
int PrintNewLine(void)
{
  // ����� ������ � ��������� OEM �� �����
  return PrintOem("\r\n");
}
/***************************************************************************/
/* LogAndPrint - ������ � ���� ������ � ����� ������ �� �����              */
/***************************************************************************/
void LogAndPrint(
  const TCHAR *pszText
  )
{
  // ������ � ���� ������
  Log(pszText);
  // ����� ������ �� �����
  Print(pszText);
}
/***************************************************************************/
/* LogAndPrintFmtV - ��������������� ������ � ���� ������ � ����� �� ����� */
/***************************************************************************/
void LogAndPrintFmtV(
  const TCHAR *pszFmt,
  va_list arglist
  )
{
  TCHAR buf[1028];
  wvsprintf(buf, pszFmt, arglist);
  // ������ � ���� ������ � ����� ������ �� �����
  LogAndPrint(buf);
}
/***************************************************************************/
/* LogAndPrintFmt - ��������������� ������ � ���� ������ � ����� �� �����  */
/***************************************************************************/
void LogAndPrintFmt(
  const TCHAR *pszFmt,
  ...
  )
{
  va_list arglist;
  va_start(arglist, pszFmt);
  // ��������������� ������ � ���� ������ � ����� �� �����
  LogAndPrintFmtV(pszFmt, arglist);
  va_end(arglist);
}
/***************************************************************************/
/* LogNewLine - ���������� � ���� ������ ����� ������ � �������            */
/*              �� ����� ������                                            */
/***************************************************************************/
void LogAndPrintNewLine(void)
{
  // ���������� � ���� ������ ����� ������
  LogNewLine();
  // ������� �� ����� ������
  PrintNewLine();
}
//---------------------------------------------------------------------------
