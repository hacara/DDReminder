#include "pch.h"

#include <winhttp.h>
#include <stdio.h>
#include ".\hwin\hwin.h"
#include "jpeg.h"

typedef struct _json_info{
	LPCWSTR name;
	LPCWSTR face_url;
	LPCWSTR live_url;
	LPCWSTR live_title;
	BOOL	live_status;
}json_info;

static HWindow hwin;

HANDLE SetMutex();
int HttpGetRequest(Buffer* buf,LPCWSTR host,LPCWSTR url);
int ExtarctJsonValue(json_info* info,LPCWSTR raw);
int SaveFaceBitmap(LPCWSTR face_url,LPCWSTR path_buf,const size_t buf_size);

DWORD CALLBACK OpenLive(LPWSTR url){
	static wchar_t script[256];
	swprintf_s(script,256,L".\\script\\init.py %s",url);

	ShellExecuteW(NULL,L"open",url,NULL,NULL,SW_SHOWNORMAL);
	ShellExecuteW(NULL,L"open",L"python.exe",script,NULL,SW_SHOWNORMAL);
	return GetLastError();
}

static wchar_t Live_Url[256];

DWORD CALLBACK StatusRetrieval(HWND hWnd,UINT _p1,UINT_PTR _p3,DWORD _p4){
	static Buffer buffer = Buffer_New();
	static wchar_t str_buf[256] = {0};
	static wchar_t icon_path[MAX_PATH] = {0};
	static BOOL notify = TRUE;
	if(!HttpGetRequest(&buffer,L"api.bilibili.com",L"/x/space/acc/info?mid=698438232\0\0\0\0\0\0\0")){
		//convert utf8 -> utf16
		wchar_t* wdata = (wchar_t*)malloc((buffer.size+1)*sizeof(wchar_t));
		wdata[buffer.size] = '\0';
		MultiByteToWideChar(CP_UTF8,0,buffer.buf,buffer.size,wdata,buffer.size);
		//check the json conrrectness and live status
		json_info info;
		if(!ExtarctJsonValue(&info,wdata)){
			swprintf_s(str_buf,256,L"正在为DD实时监控: %s 的直播间状态",info.name);
			hModifyNotifyIconTips(&hwin,str_buf);
			//automaic download uper's face-icon and convert it to bitmap file
			
			lstrcpyW(Live_Url,info.live_url);
			SaveFaceBitmap(info.face_url,icon_path,MAX_PATH);
			hModifyNotifyIconIcon(&hwin,icon_path);
			//check up uper's live status
			if(info.live_status){
				if(notify){
					//format tips info and push it to user
					swprintf_s(str_buf,256,L"你关注的%s正在直播:\n%s\n点击打开直播间",info.name,info.live_title);
					hPushLiveNotify(&hwin,L"提示",str_buf,OpenLive,Live_Url);
					notify = FALSE;
				}
			}
			else if(!notify){
				swprintf_s(str_buf,256,L"你关注的%s已下播",info.name);
				hPushLiveNotify(&hwin,L"提示",str_buf,NULL,NULL);
				notify = TRUE;
			}
		}
		free(wdata);
	}
	Buffer_Flush(&buffer);
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow){
	HANDLE hMutex = NULL;
	if(hMutex = SetMutex()){
		hwin.url = Live_Url;
		hCreateWindow(&hwin,L"NULL",hInstance);	
		hCreateNotifyIcon(&hwin);

		StatusRetrieval(NULL,0,0,0);	
		SetTimer(hwin.hWnd,UID_TIMER,10000,StatusRetrieval);
		HMsgLoop();
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
	}
	else{
		MessageBoxW(NULL,L"进程创建失败\n请检查是否重复开启",NULL,MB_OK|MB_ICONERROR);
	}
	return GetLastError();
}

HANDLE SetMutex(){
	char* file_buf = (char*)malloc(MAX_PATH);
	char* file_name = file_buf + GetModuleFileNameA(NULL,file_buf,MAX_PATH)-4;
	*file_name = '\0';
	for(;file_name[0]!='\\';file_name--);
	*file_name = '_';
	HANDLE hMutex = CreateMutexA(NULL,FALSE,file_name);
	if(GetLastError() == ERROR_ALREADY_EXISTS){
		ReleaseMutex(hMutex);
		CloseHandle(hMutex);
		hMutex = NULL;
	}
	free(file_buf);
	return hMutex;
}

int HttpGetRequest(Buffer* buf,LPCWSTR server_name,LPCWSTR url){
	HINSTANCE hSession = NULL;
	HINSTANCE hConnect = NULL;
	HINSTANCE hRequest = NULL;

	aassert(hSession = WinHttpOpen(L"http/1.0",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0));
	aassert(hConnect = WinHttpConnect(hSession,server_name,INTERNET_DEFAULT_HTTP_PORT,0));
	aassert(hRequest = WinHttpOpenRequest(hConnect,NULL,url,NULL,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,0));
	aassert(WinHttpSendRequest(hRequest,NULL,0,WINHTTP_NO_REQUEST_DATA,0,0,0));

	//ReceiveServerData
	aassert(WinHttpReceiveResponse(hRequest,0));
	DWORD read_len;
	do{
		byte* data;
		aassert(WinHttpQueryDataAvailable(hRequest,&read_len));
		aassert((data=(byte*)AllocMemory(read_len)));
		aassert(WinHttpReadData(hRequest,data,read_len,NULL));
		Buffer_Write(buf,data,read_len);
		free(data);
	}while(read_len);

	CATCH:
	WinHttpCloseHandle(hSession);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hRequest);
	return GetLastError();
}
int ExtarctJsonValue(json_info* info,LPCWSTR raw){
	wchar_t* status_start = wcsstr(raw,L"liveStatus");
	if(!status_start) return 1;

	info->live_status = status_start[12] == '1';

	info->name			= &wcsstr(raw,L",\"name\":\"")[9];
	wchar_t* name_end	= wcschr(info->name,'\"');

	info->face_url		= wcsstr(name_end,L"/bfs/face/");
	wchar_t* furl_end	= wcschr(info->face_url,'\"');

	info->live_url		= wcsstr(furl_end,L"https://live.bilibili.com/");
	wchar_t* lurl_end	= wcsstr(info->live_url,L"\",\"title\":\"");
	info->live_title	= &lurl_end[11];
	wchar_t* title_end	= wcschr(info->live_title,'\"');

	*name_end	= '\0';
	*furl_end	= '\0';
	*lurl_end 	= '\0';
	*title_end 	= '\0';

	return ERROR_SUCCESS;
}

int SaveFaceBitmap(LPCWSTR face_url,LPCWSTR path_buf,const size_t buf_size){
	swprintf_s((wchar_t* const)path_buf,buf_size,L".\\faceicons\\%s.bmp",&face_url[10]);
	CreateDirectoryW(L".\\faceicons",NULL);
	if(GetFileAttributesW(path_buf) == INVALID_FILE_ATTRIBUTES){
		Buffer img = Buffer_New();
		HttpGetRequest(&img,L"i1.hdslb.com",face_url);
		ConvertJpeg2Bitmap(&img);
		
		HANDLE hFile = CreateFileW(path_buf,GENERIC_ALL,FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    	WriteFile(hFile,img.buf,img.size,NULL,NULL);
		CloseHandle(hFile);
		Buffer_Flush(&img);
	}
	return 0;
}