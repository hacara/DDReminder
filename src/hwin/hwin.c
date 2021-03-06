#include "../pch.h"
#include "hwin.h"
#include "../../resources/resources.h"

DWORD CALLBACK hWndProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam){
    static HWindow* hWin = NULL;
    static WM_TASKBARCREATED = -1;
    switch(msg){
        case WM_CREATE:
            hWin = ((HWindow*)((LPCREATESTRUCTW)lParam)->lpCreateParams);
            WM_TASKBARCREATED = RegisterWindowMessageW(L"TaskbarCreated");
            // hWin->hMenu = CreateMenu();
            // AppendMenuW(hWin->hMenu,MF_STRING,MENU_EXIT,L"退出");
            return TRUE;
        case WM_COMMAND:
            switch(wParam){
                case UID_MENU_EXIT:
                    DestroyWindow(hWnd);
                    break;
                case UID_MENU_OPENLIVE:
	                ShellExecuteW(NULL,L"open",hWin->url,NULL,NULL,SW_SHOWNORMAL);
                    break;
            }
            return TRUE;
        case WM_DESTROY:
            KillTimer(hWnd,UID_TIMER);
            hDeleteNotifyIcon(hWin);
            PostQuitMessage(0);
            return TRUE;
        case WMAPP_NOTIFYCALLBACK:
            switch(lParam){
                case WM_CONTEXTMENU:case WM_RBUTTONDOWN:
                    {
                        HMENU popup = CreatePopupMenu();
                        AppendMenuW(popup,MF_STRING,UID_MENU_OPENLIVE,L"进入直播间");
                        AppendMenuW(popup, MF_STRING, UID_MENU_EXIT, L"退出");

                        POINT mpt;
                        GetCursorPos(&mpt);
                        SetForegroundWindow(hWnd);
                        PostMessageW(hWnd,WM_COMMAND,TrackPopupMenu(popup, TPM_RETURNCMD, mpt.x, mpt.y,0,hWnd,NULL),0);
                        DestroyMenu(popup);
                    }
                    break;
                case NIN_BALLOONUSERCLICK:
                    if(hWin->notify_shell){
                        hWin->notify_shell(hWin->shell_parameter);
                    }
                    break;   
            }
            return TRUE; 
    }
    if(msg == WM_TASKBARCREATED){
        hCreateNotifyIcon(hWin);
    }
    return DefWindowProcW(hWnd,msg,wParam,lParam);
}

int __fastcall hCreateWindow(HWindow* hWin,LPCWSTR title,HINSTANCE hInstance){
    const wchar_t* const class_name = L"HWIN_NOWND";
    WNDCLASSW wndclass;
    ZeroMemory(&wndclass,sizeof(WNDCLASSW));
    wndclass.lpfnWndProc = hWndProc;
    wndclass.hInstance = hInstance;
    wndclass.lpszClassName = class_name;

    aassert(RegisterClassW(&wndclass));

    aassert((hWin->hWnd = CreateWindowW(class_name,title,0,0,0,0,0,NULL,NULL,hInstance,hWin)));
    hWin->hInstance = hInstance;

    CATCH:
    return GetLastError();
}

int __fastcall hCreateNotifyIcon(HWindow* hWin){
    ZeroMemory(&hWin->nid,sizeof(NOTIFYICONDATAW));
    hWin->nid.cbSize = sizeof(NOTIFYICONDATAW);
    hWin->nid.hWnd = hWin->hWnd;
    hWin->nid.uID = 100;
    hWin->nid.uFlags = NIF_MESSAGE|NIF_TIP|NIF_INFO|NIF_ICON;
    hWin->nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
    hWin->nid.hIcon = LoadIconW(hWin->hInstance,MAKEINTRESOURCEW(IDI_ICON1));
    hWin->nid.uVersion = NOTIFYICON_VERSION_4;
    hWin->nid.dwInfoFlags = NIIF_USER;

    return Shell_NotifyIconW(NIM_ADD,&hWin->nid);
}

int __fastcall hModifyNotifyIconTips(HWindow* hWin,LPCWSTR tips_str){
    hWin->nid.uFlags = NIF_ICON|NIF_TIP;
    lstrcpyW(hWin->nid.szTip,tips_str);
    return Shell_NotifyIconW(NIM_MODIFY,&hWin->nid);
}

int __fastcall hModifyNotifyIconIcon(HWindow* hWin,LPCWSTR icon_path){
    DeleteObject(hWin->nid.hIcon);
    hWin->nid.uFlags = NIF_ICON|NIF_TIP;
    HBITMAP hBitmap = LoadImageW(NULL,icon_path,IMAGE_BITMAP,512,512,LR_DEFAULTSIZE|LR_LOADFROMFILE);
    ICONINFO ii;
    ii.fIcon = TRUE;
    ii.hbmColor = hBitmap;
    ii.hbmMask = hBitmap;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    hWin->nid.hIcon = CreateIconIndirect(&ii);
    DeleteObject(hBitmap);
    return Shell_NotifyIconW(NIM_MODIFY,&hWin->nid);
}

int __fastcall hDeleteNotifyIcon(HWindow* hWin){
    DeleteObject(hWin->nid.hIcon);
    return Shell_NotifyIconW(NIM_DELETE,&hWin->nid);
}

int __fastcall hPushLiveNotify(HWindow* hWin,LPCWSTR info_title,LPCWSTR info_str,LPTHREAD_START_ROUTINE shell,LPVOID shell_parameter){
    lstrcpyW((LPWSTR)&hWin->nid.szInfoTitle,info_title);
    lstrcpyW((LPWSTR)&hWin->nid.szInfo,info_str);
    
    hWin->nid.uFlags = NIF_MESSAGE|NIF_TIP|NIF_INFO|NIF_ICON;
    hWin->notify_shell = shell;
    hWin->shell_parameter = shell_parameter;
    return Shell_NotifyIconW(NIM_MODIFY,&hWin->nid);
}