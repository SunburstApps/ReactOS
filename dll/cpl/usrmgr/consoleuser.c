/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS User Manager Control Panel
 * FILE:            dll/cpl/usrmgr/userprops.c
 * PURPOSE:         CONSOLE-user property sheet
 */

#include "usrmgr.h"
#include <secext.h>

static LPWSTR GetCurrentUserName(VOID)
{
    ULONG size = 0;
    BOOLEAN ok = GetUserNameEx(NameSamCompatible, NULL, &size);
    if (ok) __fastfail(0); // This should never happen

    if (GetLastError() == ERROR_MORE_DATA)
    {
        LPWSTR buffer = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, size * sizeof(WCHAR));
        ok = GetUserNameEx(NameSamCompatible, buffer, &size);
        if (ok) return buffer;
        
        LocalFree(buffer);
        return NULL;
    }
    else
    {
        // Some other failure, leave it in GetLastError() for the caller to handle
        return NULL;
    }
}

static LPWSTR AllocAndCopyWindowText(HWND hWnd)
{
    UINT length = GetWindowTextLengthW(hWnd) + 1;
    LPWSTR buffer = LocalAlloc(LMEM_ZEROINIT, length * sizeof(WCHAR));
    GetWindowTextW(hWnd, buffer, length);
    return buffer;
}

static INT_PTR CALLBACK ConsoleUserChangePasswordDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
            EnableWindow(GetDlgItem(hwndDlg, IDOK), FALSE);
        }; break;

        case WM_COMMAND:
        {
            if (LOWORD(wParam) == IDOK)
            {
                HWND hwndOldPassword = GetDlgItem(hwndDlg, IDC_EDIT_OLDPASSWORD);
                HWND hwndNewPassword = GetDlgItem(hwndDlg, IDC_EDIT_PASSWORD1);
                if (CheckPasswords(hwndDlg, IDC_EDIT_PASSWORD1, IDC_EDIT_PASSWORD2))
                {
                    LPWSTR userName = (LPWSTR)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
                    LPWSTR oldPassword = AllocAndCopyWindowText(hwndOldPassword);
                    LPWSTR newPassword = AllocAndCopyWindowText(hwndNewPassword);

                    NET_API_STATUS status = NetUserChangePassword(NULL, userName, oldPassword, newPassword);
                    if (status == NERR_Success)
                    {
                        EndDialog(hwndDlg, IDOK);
                        return FALSE;
                    }

                    LPWSTR errorMessage = L"Could not change password!";
                    if (status == ERROR_ACCESS_DENIED)
                    {
                        errorMessage = L"Access denied.";
                    }
                    else if (status == ERROR_INVALID_PASSWORD)
                    {
                        errorMessage = L"Old password is incorrect.";
                    }
                    else if (status == NERR_UserNotFound)
                    {
                        errorMessage = L"User not found.";
                    }
                    else if (status == NERR_PasswordTooShort)
                    {
                        errorMessage = L"New password does not meet applicable password policies.";
                    }

                    MessageBoxW(hwndDlg, errorMessage, L"ERROR", MB_ICONERROR | MB_OK);
                }
            }
            else if (LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hwndDlg, IDCANCEL);
            }
            else if (LOWORD(wParam) == IDC_EDIT_PASSWORD1 || LOWORD(wParam) == IDC_EDIT_PASSWORD2)
            {
                int p1Length = SendDlgItemMessage(hwndDlg, IDC_EDIT_PASSWORD1, WM_GETTEXTLENGTH, 0, 0);
                int p2Length = SendDlgItemMessage(hwndDlg, IDC_EDIT_PASSWORD2, WM_GETTEXTLENGTH, 0, 0);

                BOOL enable = (p1Length > 0) && (p2Length > 0);
                EnableWindow(GetDlgItem(hwndDlg, IDOK), enable);
            }
        }; break;
    }

    return FALSE;
}

INT_PTR CALLBACK ConsoleUserPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPWSTR userName = GetCurrentUserName();

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            if (userName != NULL)
            {
                SetDlgItemTextW(hwndDlg, IDC_CONSOLE_NAME_DIALOG_CAPTION, userName);

                // TODO: Add support for Power Users, domain accounts
                DWORD stringId = IsUserAnAdmin() ? IDS_LOCAL_ADMINISTRATOR_DESCRIPTION : IDS_LOCAL_REGULARUSER_DESCRIPTION;
                LPWSTR string = NULL;
                AllocAndLoadString(&string, hApplet, stringId);

                if (string != NULL)
                {
                    SetDlgItemTextW(hwndDlg, IDC_CONSOLE_NAME_DIALOG_DESCRIPTION_LABEL, string);
                    LocalFree(string);
                }
            }
            else
            {
                LPWSTR errorMessage = NULL, title = NULL;
                AllocAndLoadString(&errorMessage, hApplet, IDS_COULD_NOT_GET_USERNAME_ERROR_MSG);
                AllocAndLoadString(&title, hApplet, IDS_CPLNAME);
                MessageBoxW(hwndDlg, errorMessage, title, MB_ICONERROR | MB_OK);
                SetDlgItemTextW(hwndDlg, IDC_CONSOLE_NAME_DIALOG_CAPTION, errorMessage);
                if (errorMessage != NULL) LocalFree(errorMessage);
                if (title != NULL) LocalFree(title);

                EnableWindow(GetDlgItem(hwndDlg, IDC_CONSOLE_USER_CHANGE_BUTTON), FALSE);
            }

            HWND hwndMainLabel = GetDlgItem(hwndDlg, IDC_CONSOLE_NAME_DIALOG_CAPTION);
            LOGFONT lf;
            ZeroMemory(&lf, sizeof(LOGFONT));
            HFONT font = (HFONT)SendMessage(hwndMainLabel, WM_GETFONT, 0, 0);
            GetObject(font, sizeof(lf), &lf);
            lf.lfWeight = FW_BOLD;
            font = CreateFontIndirect(&lf);
            SendMessage(hwndMainLabel, WM_SETFONT, (WPARAM)font, (LPARAM)TRUE);
            SetWindowLongPtr(hwndMainLabel, GWLP_USERDATA, (LONG_PTR)font);
        }; break;

        case WM_DESTROY:
        {
            HWND hwndMainLabel = GetDlgItem(hwndDlg, IDC_CONSOLE_NAME_DIALOG_CAPTION);
            HFONT font = (HFONT)GetWindowLongPtr(hwndMainLabel, GWLP_USERDATA);
            DeleteObject(font);
        }; break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                case IDC_CONSOLE_USER_CHANGE_BUTTON:
                {
                    INT_PTR result = DialogBoxParamW(hApplet, MAKEINTRESOURCE(IDD_CONSOLE_USER_CHANGEPASSWORD), hwndDlg, ConsoleUserChangePasswordDlgProc, (LPARAM)userName);
                    if (result == IDOK) PropSheet_CancelToClose(GetParent(hwndDlg));
                }
                break;
            }
        }; break;
    }

    if (userName != NULL) LocalFree(userName);
    return FALSE;
}
