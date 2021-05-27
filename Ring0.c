/***************************************************************************/
/* Ring0.C - ���������� ���� � ������� ������ (Ring 0)                     */
/* ������ 1.01 (��� 2002 �.)                                               */
/*                                                                         */
/* OS: Windows 9x                                                          */
/*                                                                         */
/* Copyright (c) 2002 rivitna                                              */
/***************************************************************************/
/*-------------------------------------------------------------------------*/
#define WIN32_LEAN_AND_MEAN  /* Exclude rarely-used stuff from Windows headers */
#include <windows.h>
#include "Ring0.h"
/*-------------------------------------------------------------------------*/
/***************************************************************************/
/* CallRing0 - ����� ��������� � ������� ������ Ring 0 (Windows 9x)        */
/***************************************************************************/

__declspec(naked) void Ring0FarFunc()
{
  __asm
  {
#ifdef __BORLANDC__
    call  ecx
#else
    call  eax
#endif  // __BORLANDC__
    retf
  }
}

__declspec(naked) BOOL __stdcall CallRing0(
  PRING0FUNC pRing0Func,
  DWORD dwParam,
  LPDWORD lpdwRetValue
  )
{
  __asm
  {
    push  ebp
    mov   ebp,esp
    push  esi

    /* ���������� ������� ����� LDT */
    sldt  ax                 /* AX = �������� LDT */
    and   eax,0FFF8h
    jz    NoFreeEntries

    push  esi
    sgdt  FWORD PTR [esp-2]
    pop   esi                /* ESI = ������� ����� GDT */
    add   esi,eax

    movzx ecx,WORD PTR [esi]
    inc   ecx
    shr   ecx,3              /* ECX = ����� ������������ � LDT */
    mov   edx,[esi+1]
    mov   dl,[esi+7]
    ror   edx,8              /* EDX = ������� ����� LDT */
    mov   esi,edx

    /* ���� ��������� ���������� � LDT */
EntryLoop:
    cmp   BYTE PTR [esi+5],0
    je    FoundFreeEntry
    add   esi,8
    loop  EntryLoop

NoFreeEntries:
    xor   eax,eax
    jmp   Done

FoundFreeEntry:
    /* ������ ��������� ���������� LDT */

    /* ����������� ���������� LDT � ���� ������ */
    mov   eax,OFFSET Ring0FarFunc
    mov   DWORD PTR [esi],eax
    mov   DWORD PTR [esi+4],eax
    mov   DWORD PTR [esi+2],0EC000028h

    /* �������� ��������� ����� ���� ������ */
    mov   eax,esi
    sub   eax,edx
    or    al,7               /* AX = �������� ����� ������ */
    push  ax
    push  eax
#ifdef __BORLANDC__
    mov   eax,[dwParam]
    mov   ecx,[pRing0Func]
    DB    0FFh, 1Ch, 24h
#else
    mov   ecx,[dwParam]
    mov   eax,[pRing0Func]
    call  FWORD PTR [esp]
#endif  // __BORLANDC__
    add   esp,6
    mov   ecx,[lpdwRetValue]
    jecxz FreeEntry
    mov   DWORD PTR [ecx],eax

FreeEntry:
    /* ����������� ���������� LDT */
    xor   eax,eax
    mov   DWORD PTR [esi],eax
    mov   DWORD PTR [esi+4],eax

    inc   eax

Done:
    pop   esi
    pop   ebp
    ret   12
  }
}
/*-------------------------------------------------------------------------*/
