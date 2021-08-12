#include <windows.h>
#include <stdio.h>

static const char MID[] = {0x6D,0x00,0x69,0x00,0x64,0x00,0x3D};

int wmain(int argc,wchar_t** argv){
    wchar_t new_mid[17] = {0};
    wchar_t path[MAX_PATH];
    wchar_t output_path[MAX_PATH];
    if(argc != 3 || lstrlenW(argv[1]) > 16){
        printf("UID:\n");
        size_t uid;
        if (!wscanf(L"%u", &uid)) {
            fprintf(stderr,"Please type a vaild number\n");
        }
        swprintf_s(new_mid,sizeof(new_mid), L"%u", uid);
        swprintf_s(output_path, sizeof(output_path), L"%u.exe", uid);
    }
    else {
        lstrcpyW(new_mid, argv[1]);
        lstrcpyW(output_path, argv[2]);
    }
    lstrcpyW(path,argv[0]);
    {
        int path_len = lstrlenW(path);
        int index = path_len;
        for(;index;index--){
            if(path[index] == '\\') break;
        }
        memcpy(&path[index+1],L"Template.dat",sizeof(L"Template.dat"));
        path[index+13] = '\0';
    }

    HANDLE hFile = CreateFileW(path,GENERIC_ALL,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        fprintf(stderr,"Template open failed\nerror code : %d\n",GetLastError());
        return -1;
    }
    
    size_t size = GetFileSize(hFile,NULL);
    byte* buf = (byte*)malloc(size);
    ReadFile(hFile,buf,size,&size,NULL);
    int index;
    for(index = size - sizeof(MID);index;index--){
        if(memcmp(buf+index,MID,sizeof(MID)) == 0) break;
    }
    if(!index){
        fprintf(stderr,"Template was broken\n");
        return -1;
    }
    
    memcpy(buf + index + sizeof(MID) + 1,new_mid,sizeof(new_mid));

    CloseHandle(hFile);

    HANDLE hNewFile = CreateFileW(output_path,GENERIC_ALL,FILE_SHARE_WRITE,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hNewFile == INVALID_HANDLE_VALUE){
        fprintf(stderr,"Create new file failed\nError Code: %d\n",GetLastError());
        return -1;
    }
    WriteFile(hNewFile,buf,size,&index,NULL);
    free(buf);
    CloseHandle(hNewFile);
    return 0;
}