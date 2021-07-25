#pragma once

#pragma comment(lib,"user32.lib")
#pragma comment(lib,"ucrt.lib")
#pragma comment(lib,"winhttp.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"gdi32.lib")

#define _DEBUG

#include <windows.h>
#include "./Buffer/Buffer.h"

inline void* AllocMemory(size_t _size){
    void* _buf = malloc(_size);
    if(!_buf){
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);   
    }
    return _buf;
}

#define assert(x) if((x)) goto CATCH
#define aassert(x) if(!(x)) goto CATCH
#define expect(x,err) if(!(x)) MessageBoxW(NULL,err,NULL,MB_OK|MB_ICONERROR)