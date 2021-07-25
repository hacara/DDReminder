#include <windows.h>
#include <stdio.h>

static const char MID[] = {0x6D,0x00,0x69,0x00,0x64,0x00,0x3D};

int wmain(int argc,wchar_t** argv){
    wchar_t new_mid[17] = {0};
    if(argc != 3 || lstrlenW(argv[1]) > 16){
        fprintf(stderr,"Wrong command line arguments");
        printf("usage: generator.exe <MID number> <Output file name>");
        return -1;
    }
    lstrcpyW(new_mid,argv[1]);

    HANDLE hFile = CreateFileA(".\\Template",GENERIC_ALL,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hFile == INVALID_HANDLE_VALUE){
        fprintf(stderr,"Template open failed");
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
        fprintf(stderr,"Template was broken");
        return -1;
    }
    
    memcpy(buf + index + sizeof(MID) + 1,new_mid,sizeof(new_mid));
    CloseHandle(hFile);

    HANDLE hNewFile = CreateFileW(argv[2],GENERIC_ALL,FILE_SHARE_WRITE,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    if(hNewFile == INVALID_HANDLE_VALUE){
        fprintf(stderr,"Create new file failed\nError Code: %d",GetLastError());
        return -1;
    }
    WriteFile(hNewFile,buf,size,&index,NULL);
    free(buf);
    CloseHandle(hNewFile);
    return 0;
}