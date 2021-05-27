//---------------------------------------------------------------------------
#ifndef __APPUTILS_H__
#define __APPUTILS_H__
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
//---------------------------------------------------------------------------
// �������� ����� ������
BOOL OpenLog(
  const TCHAR *pszFileName,
  BOOL bAppend
  );
// �������� ����� ������
void CloseLog(void);
// ������ � ���� ������
int Log(
  const TCHAR *pszMsg
  );
// ��������������� ������ � ���� ������
int LogFmtV(
  const TCHAR *pszFmt,
  va_list arglist
  );
// ��������������� ������ � ���� ������
int LogFmt(
  const TCHAR *pszFmt,
  ...
  );
// ���������� � ���� ������ ����� ������
int LogNewLine(void);
// ����� ������ � ��������� OEM �� �����
int PrintOem(
  const char *pszOemText
  );
// ����� ������ �� �����
int Print(
  const TCHAR *pszText
  );
// ��������������� ����� ������ �� �����
int PrintFmtV(
  const TCHAR *pszFmt,
  va_list arglist
  );
// ��������������� ����� ������ �� �����
int PrintFmt(
  const TCHAR *pszFmt,
  ...
  );
// ������� �� ����� ������
int PrintNewLine(void);
// ������ � ���� ������ � ����� ������ �� �����
void LogAndPrint(
  const TCHAR *pszText
  );
// ��������������� ������ � ���� ������ � ����� �� �����
void LogAndPrintFmtV(
  const TCHAR *pszFmt,
  va_list arglist
  );
// ��������������� ������ � ���� ������ � ����� �� �����
void LogAndPrintFmt(
  const TCHAR *pszFmt,
  ...
  );
// ���������� � ���� ������ ����� ������ � ������� �� ����� ������
void LogAndPrintNewLine(void);
//---------------------------------------------------------------------------
#ifdef __cplusplus
}  // extern "C"
#endif // __cplusplus
//---------------------------------------------------------------------------
#endif // __APPUTILS_H__
