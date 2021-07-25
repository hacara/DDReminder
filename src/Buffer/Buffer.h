#pragma once

typedef struct _buffer{
    byte* buf;
    size_t size;
}Buffer;

#define Buffer_New() {0,0}
BOOL __fastcall Buffer_Write(Buffer* buffer,void* data,size_t size);
void __fastcall Buffer_Flush(Buffer* buffer);