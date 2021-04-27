/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS User Manager Control Panel
 * FILE:            dll/cpl/usrmgr/misc.c
 * PURPOSE:         Miscellaneous functions
 *
 * PROGRAMMERS:     Eric Kohl
 */

#include "usrmgr.h"
#include <stdarg.h>

VOID
DebugPrintf(LPTSTR szFormat, ...)
{
    TCHAR szOut[512];
    va_list arg_ptr;


    va_start (arg_ptr, szFormat);
    _vstprintf (szOut, szFormat, arg_ptr);
    va_end (arg_ptr);

    MessageBox(NULL, szOut, _T("Debug"), MB_OK);
}

BOOL
CheckAccountName(HWND hwndDlg,
                 INT nIdDlgItem,
                 LPTSTR lpAccountName)
{
    TCHAR szAccountName[256];
    UINT uLen;

    if (lpAccountName)
        uLen = _tcslen(lpAccountName);
    else
        uLen = GetDlgItemText(hwndDlg, nIdDlgItem, szAccountName, 256);

    /* Check the account name */
    if (uLen > 0 &&
        _tcspbrk((lpAccountName) ? lpAccountName : szAccountName, TEXT("\"*+,/\\:;<=>?[]|")) != NULL)
    {
        MessageBox(hwndDlg,
                   TEXT("The account name you entered is invalid! An account name must not contain the following characters: *+,/:;<=>?[\\]|"),
                   TEXT("ERROR"),
                   MB_OK | MB_ICONERROR);
        return FALSE;
    }

    return TRUE;
}

static INT
LengthOfStrResource(IN HINSTANCE hInst,
                    IN UINT uID)
{
    HRSRC hrSrc;
    HGLOBAL hRes;
    LPWSTR lpName, lpStr;

    if (hInst == NULL)
    {
        return -1;
    }

    /* There are always blocks of 16 strings */
    lpName = (LPWSTR)MAKEINTRESOURCE((uID >> 4) + 1);

    /* Find the string table block */
    if ((hrSrc = FindResourceW(hInst, lpName, (LPWSTR)RT_STRING)) &&
        (hRes = LoadResource(hInst, hrSrc)) &&
        (lpStr = (WCHAR*) LockResource(hRes)))
    {
        UINT x;

        /* Find the string we're looking for */
        uID &= 0xF; /* position in the block, same as % 16 */
        for (x = 0; x < uID; x++)
        {
            lpStr += (*lpStr) + 1;
        }

        /* Found the string */
        return (int)(*lpStr);
    }
    return -1;
}

INT
AllocAndLoadString(OUT LPWSTR *lpTarget,
                   IN HINSTANCE hInst,
                   IN UINT uID)
{
    INT ln;

    ln = LengthOfStrResource(hInst,
                             uID);
    if (ln++ > 0)
    {
        (*lpTarget) = (LPWSTR)LocalAlloc(LMEM_FIXED,
                                         ln * sizeof(WCHAR));
        if ((*lpTarget) != NULL)
        {
            INT Ret;
            if (!(Ret = LoadStringW(hInst, uID, *lpTarget, ln)))
            {
                LocalFree((HLOCAL)(*lpTarget));
            }
            return Ret;
        }
    }
    return 0;
}

DWORD
LoadAndFormatString(OUT LPWSTR *lpTarget,
                    IN HINSTANCE hInstance,
                    IN UINT uID,
                    ...)
{
    DWORD Ret = 0;
    LPWSTR lpFormat;
    va_list lArgs;

    if (AllocAndLoadString(&lpFormat,
                           hInstance,
                           uID) > 0)
    {
        va_start(lArgs, uID);
        /* let's use Format to format it because it has the ability to allocate
           memory automatically */
        Ret = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                             lpFormat,
                             0,
                             0,
                             (LPWSTR)lpTarget,
                             0,
                             &lArgs);
        va_end(lArgs);

        LocalFree((HLOCAL)lpFormat);
    }

    return Ret;
}

VOID
ResourceMessageBox(
    HINSTANCE hInstance,
    HWND hwnd,
    UINT uType,
    UINT uCaptionId,
    UINT uMessageId)
{
    LPWSTR szErrorText = NULL, szErrorCaption = NULL;
    AllocAndLoadString(&szErrorCaption, hInstance, uCaptionId);
    AllocAndLoadString(&szErrorText, hInstance, uMessageId);

    MessageBoxW(hwnd, szErrorText, szErrorCaption, uType);

    if (szErrorCaption != NULL) LocalFree(szErrorCaption);
    if (szErrorText != NULL) LocalFree(szErrorText);
}
