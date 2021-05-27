//---------------------------------------------------------------------------
#ifndef __STDUTILS_H__
#define __STDUTILS_H__
//---------------------------------------------------------------------------
#include <tchar.h>
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus
//---------------------------------------------------------------------------
#ifndef _USE_STDLIB
#undef malloc
#define malloc      AllocMem
#undef realloc
#define realloc     ReAllocMem
#undef calloc
#define calloc      AllocArray
#undef free
#define free        FreeMem
#undef memset
#define memset      FillMem
#undef FillMemory
#define FillMemory  FillMem
#undef wmemset
#define wmemset     FillMemW
#undef ZeroMemory
#define ZeroMemory  ZeroMem
#undef memcpy
#define memcpy      CopyMem
#undef CopyMemory
#define CopyMemory  CopyMem
#undef memmove
#define memmove     MoveMem
#undef MoveMemory
#define MoveMemory  MoveMem
#undef mem�hr
#define memchr      MemChr
#undef memcmp
#define memcmp(buf1, buf2, count)  (!EqualMem((buf1), (buf2), (count)))
#endif  // _USE_STDLIB
//---------------------------------------------------------------------------
// ��������� ������ ������
void *AllocMem(
  size_t size
  );
// ��������� ������� ������ ������
void *ReAllocMem(
  void *memblock,
  size_t size
  );
// ��������� ������ ��� ������� ��������� � ������������� �� ��������
// ����������
void *AllocArray(
  size_t num,
  size_t size
  );
// ������������ ����������� ������ ������
void FreeMem(
  void *memblock
  );
// ���������� ������
void*
#ifndef _WIN64
__stdcall
#endif  // _WIN64
FillMem(
  void *dest,
  unsigned char c,
  size_t count
  );
// ���������� ������
wchar_t*
#ifndef _WIN64
__stdcall
#endif  // _WIN64
FillMemW(
  wchar_t *dest,
  wchar_t c,
  size_t count
  );
// ��������� ������
void __fastcall ZeroMem(
  void *dest,
  size_t count
  );
// ����������� ����� ������
void*
#ifndef _WIN64
__stdcall
#endif  // _WIN64
CopyMem(
  void *dest,
  const void *src,
  size_t count
  );
// ����������� ����� ������
void*
#ifndef _WIN64
__stdcall
#endif  // _WIN64
MoveMem(
  void *dest,
  const void *src,
  size_t count
  );
// ��������� ������ ������
int /* BOOL */
#ifndef _WIN64
__stdcall
#endif  // _WIN64
EqualMem(
  const void *buf1,
  const void *buf2,
  size_t count
  );
// ����� ����� � ����� ������
void*
#ifndef _WIN64
__stdcall
#endif  // _WIN64
MemChr(
  const void *buf,
  int c,
  size_t count
  );
// ����� ������� ������ � ����� ������
void*
#ifndef _WIN64
__stdcall
#endif  // _WIN64
SearchMem(
  const void *buf,
  size_t buflen,
  const void *s,
  size_t slen
  );
#ifndef _WIN64
// ��������� 64-��������� �����
__int64 __stdcall Mul64(
  __int64 multiplier,
  __int64 multiplicand
  );
// ������� 64-��������� ����������� �����
unsigned __int64 __stdcall DivUI64(
  unsigned __int64 dividend,
  unsigned __int64 divisor
  );
// ��������� ������� �� ������� 64-��������� ����������� �����
unsigned __int64 __stdcall RemUI64(
  unsigned __int64 dividend,
  unsigned __int64 divisor
  );
#endif  // _WIN64
//---------------------------------------------------------------------------
#ifdef __cplusplus
}  // extern "C"
#endif // __cplusplus
//---------------------------------------------------------------------------
#endif  // __STDUTILS_H__
