#pragma once

#include <windows.h>

#define WNDPROCHEAD() \
    DWORD CALLBACK WNDPROC(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) { \
        switch(hWnd) {
#define WNDPROCEND() \
        } \
        return DefWindowProcW(hWnd,msg,wParam,lParam); \
    }

typedef struct _hwin{
    HINSTANCE hInstance;
    HWND hWnd;
    HMENU hMenu;
    LPCWSTR url;
    LPTHREAD_START_ROUTINE notify_shell;
    PVOID shell_parameter;
    NOTIFYICONDATAW nid;
}HWindow;

int __fastcall hCreateWindow(HWindow* hWin,LPCWSTR title,HINSTANCE hInstance);

int __fastcall hCreateNotifyIcon(HWindow* hWin);

int __fastcall hModifyNotifyIconTips(HWindow* hWin,LPCWSTR tips_str);

int __fastcall hModifyNotifyIconIcon(HWindow* hWin,LPCWSTR icon_path);
    
int __fastcall hDeleteNotifyIcon(HWindow* hWin);

int __fastcall hPushLiveNotify(HWindow* hWin,LPCWSTR info_title,LPCWSTR info_str,LPTHREAD_START_ROUTINE shell,LPVOID shell_paramter);
    
inline void HMsgLoop(){
    MSG msg;
    while(GetMessageW(&msg,NULL,0,0)){
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

#define WMAPP_NOTIFYCALLBACK WM_APP + 1
#define UID_MENU_EXIT 101
#define UID_MENU_OPENLIVE 102
#define UID_TIMER 1001