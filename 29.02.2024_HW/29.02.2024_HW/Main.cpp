#include <windows.h>
#include <windowsX.h>
#include <tchar.h>
#include <string>
#include "resource.h"
#include <fstream>
#include <commctrl.h>

using namespace std;

static HWND hChildWnd1 = NULL;
HWND hSpin1, hSpin2, hSpin3;
HICON hIcon;

INT_PTR CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp);
DWORD WINAPI Thread(LPVOID lp);

INT_PTR CALLBACK ChildDlgProc1(HWND hChildWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrev, LPTSTR lpszCmdLine, int nCmdShow)
{
    return DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, (DLGPROC)DlgProc);
}

INT_PTR CALLBACK DlgProc(HWND hWnd, UINT mes, WPARAM wp, LPARAM lp)
{
    switch (mes)
    {
    case WM_INITDIALOG:
    {
        HINSTANCE hInst = GetModuleHandle(NULL);
        hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
        SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);


        hSpin1 = GetDlgItem(hWnd, IDC_SPIN1);
        SendMessage(hSpin1, UDM_SETRANGE32, 0, 23);
        SendMessage(hSpin1, UDM_SETBUDDY, WPARAM(GetDlgItem(hWnd, IDC_EDIT2)), 0);
        SetWindowText(GetDlgItem(hWnd, IDC_EDIT2), TEXT("12"));

        hSpin2 = GetDlgItem(hWnd, IDC_SPIN2);
        SendMessage(hSpin2, UDM_SETRANGE32, 0, 59);
        SendMessage(hSpin2, UDM_SETBUDDY, WPARAM(GetDlgItem(hWnd, IDC_EDIT3)), 0);
        SetWindowText(GetDlgItem(hWnd, IDC_EDIT3), TEXT("30"));

        hSpin3 = GetDlgItem(hWnd, IDC_SPIN3);
        SendMessage(hSpin3, UDM_SETRANGE32, 0, 59);
        SendMessage(hSpin3, UDM_SETBUDDY, WPARAM(GetDlgItem(hWnd, IDC_EDIT4)), 0);
        SetWindowText(GetDlgItem(hWnd, IDC_EDIT4), TEXT("30"));
    }
    break;
    case WM_COMMAND:
    {
        switch (LOWORD(wp))
        {
        case IDC_BUTTON1:
        {
            TCHAR szText1[50], szText2[5], szText3[5], szText4[5];
            GetWindowText(GetDlgItem(hWnd, IDC_EDIT1), szText1, 50);
            GetWindowText(GetDlgItem(hWnd, IDC_EDIT2), szText2, 5);
            GetWindowText(GetDlgItem(hWnd, IDC_EDIT3), szText3, 5);
            GetWindowText(GetDlgItem(hWnd, IDC_EDIT4), szText4, 5);

            if (_tcslen(szText1) > 0 && _tcslen(szText2) > 0 && _tcslen(szText3) > 0 && _tcslen(szText4) > 0)
            {
                std::wstring listItem = szText1;
                listItem += L" - ";
                listItem += szText2;
                listItem += L":";
                listItem += szText3;
                listItem += L":";
                listItem += szText4;

                HWND hList = GetDlgItem(hWnd, IDC_LIST1);
                SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)listItem.c_str());

                SetWindowText(GetDlgItem(hWnd, IDC_EDIT1), TEXT(""));
                SetWindowText(GetDlgItem(hWnd, IDC_EDIT2), TEXT("12"));
                SetWindowText(GetDlgItem(hWnd, IDC_EDIT3), TEXT("30"));
                SetWindowText(GetDlgItem(hWnd, IDC_EDIT4), TEXT("30"));

                HANDLE h;
                h = CreateThread(NULL, 0, Thread, hWnd, 0, NULL);
                CloseHandle(h);
            }
            else
            {
                SetWindowText(GetDlgItem(hWnd, IDC_EDIT2), TEXT("12"));
                SetWindowText(GetDlgItem(hWnd, IDC_EDIT3), TEXT("30"));
                SetWindowText(GetDlgItem(hWnd, IDC_EDIT4), TEXT("30"));

                MessageBox(hWnd, TEXT("Enter values in all fields"), TEXT("Error"), MB_OK | MB_ICONERROR);
            }
        }
        break;
        }
    }
    break;
    case WM_CLOSE:
    {
        EndDialog(hWnd, 0);
        return TRUE;
    }
    }
    return FALSE;
}

DWORD WINAPI Thread(LPVOID lp)
{
    HWND hWnd = (HWND)lp;

    TCHAR buf[10];
    GetWindowText(GetDlgItem(hWnd, IDC_EDIT2), buf, 10);
    int hours = _tstoi(buf);
    GetWindowText(GetDlgItem(hWnd, IDC_EDIT3), buf, 10);
    int minutes = _tstoi(buf);
    GetWindowText(GetDlgItem(hWnd, IDC_EDIT4), buf, 10);
    int seconds = _tstoi(buf);

    SYSTEMTIME st;
    GetLocalTime(&st);

    int diffHours = hours - st.wHour;
    int diffMinutes = minutes - st.wMinute;
    int diffSeconds = seconds - st.wSecond;

    if (diffHours >= 0 || (diffHours == 0 && diffMinutes >= 0) || (diffHours == 0 && diffMinutes == 0 && diffSeconds >= 0)) {
        DWORD milliseconds = (diffHours * 3600 + diffMinutes * 60 + diffSeconds) * 1000;

        HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
        if (hTimer == NULL) {
            MessageBox(hWnd, TEXT("Error when creating a timer"), TEXT("Error"), MB_OK | MB_ICONERROR);
            return 1;
        }

        LARGE_INTEGER liDueTime;
        liDueTime.QuadPart = -static_cast<LONGLONG>(milliseconds) * 10000;
        if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, FALSE)) {
            MessageBox(hWnd, TEXT("Error when setting the timer"), TEXT("Error"), MB_OK | MB_ICONERROR);
            CloseHandle(hTimer);
            return 1;
        }

        if (WaitForSingleObject(hTimer, INFINITE) == WAIT_OBJECT_0)
        {
            if (hChildWnd1 == NULL)
            {
                hChildWnd1 = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DIALOG2), hWnd, (DLGPROC)ChildDlgProc1);

                if (hChildWnd1 != NULL)
                {
                    ShowWindow(hChildWnd1, SW_SHOW);
                }
            }
            else
            {
                ShowWindow(hChildWnd1, SW_SHOW);
            }

            for (int i = 0; i < 5; i++)
            {
                Beep(1000, 500);
                Sleep(100);
            }
        }

        CancelWaitableTimer(hTimer);
        CloseHandle(hTimer);
    }

    return 0;
}



INT_PTR CALLBACK ChildDlgProc1(HWND hChildWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDC_BUTTON1:
        {
           
        }
        break;
        }
    }
    break;
    case WM_CLOSE:
        EndDialog(hChildWnd, 0);
        hChildWnd = NULL;
        return FALSE;
    }
    return FALSE;
}