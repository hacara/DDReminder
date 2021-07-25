#include "../pch.h"
#include "Buffer.h"


int __fastcall Buffer_Write(Buffer* buffer,void* data,size_t size){
    byte* new_buffer;

    if(!(new_buffer=AllocMemory(buffer->size+size))) return GetLastError();

    if(buffer->buf){
        memcpy(new_buffer,buffer->buf,buffer->size);
        free(buffer->buf);
    }
    memcpy(new_buffer+buffer->size,data,size);

    buffer->buf = new_buffer;
    buffer->size += size;
    return ERROR_SUCCESS;
}

void __fastcall Buffer_Flush(Buffer* buffer){
    if(buffer->buf) free(buffer->buf);
    buffer->buf = NULL;
    buffer->size = 0;
}