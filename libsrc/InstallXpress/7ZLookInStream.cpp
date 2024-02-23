#include "stdafx.h"
#include "7zLookInStream.h"

SRes Mem7zLookInStream_Look(ILookInStreamPtr p, const void** buf, size_t* size) {
    CMem7zLookInStream* stream = (CMem7zLookInStream*)p;
    *size = (size_t)(stream->size - stream->position);
    *buf = stream->data + stream->position;
    return SZ_OK;
}

SRes Mem7zLookInStream_Skip(ILookInStreamPtr p, size_t offset) {
    CMem7zLookInStream* stream = (CMem7zLookInStream*)p;
    if (stream->position + offset > stream->size) {
        return SZ_ERROR_READ; // Ô½½ç
    }
    stream->position += offset;
    return SZ_OK;
}

SRes Mem7zLookInStream_Read(ILookInStreamPtr p, void* buf, size_t* size) {
    CMem7zLookInStream* stream = (CMem7zLookInStream*)p;
    size_t readable = (size_t)(stream->size - stream->position);
    size_t toRead = (*size < readable) ? *size : readable;
    memcpy(buf, stream->data + stream->position, toRead);
    stream->position += toRead;
    *size = toRead;
    return SZ_OK;
}

SRes Mem7zLookInStream_Seek(ILookInStreamPtr p, Int64* pos, ESzSeek origin) {
    CMem7zLookInStream* stream = (CMem7zLookInStream*)p;
    Int64 newPos = *pos;

    switch (origin) {
    case SZ_SEEK_SET:
        break;
    case SZ_SEEK_CUR:
        newPos = stream->position + newPos;
        break;
    case SZ_SEEK_END:
        newPos = stream->size + newPos;
        break;
    default:
        return SZ_ERROR_PARAM;
    }

    if (newPos < 0 || newPos > (Int64)stream->size) {
        return SZ_ERROR_PARAM;
    }

    stream->position = newPos;
    *pos = newPos;
    return SZ_OK;
}

// ILookInStream vtable
static const ILookInStream Mem7zLookInStreamVtbl = {
    Mem7zLookInStream_Look,
    Mem7zLookInStream_Skip,
    Mem7zLookInStream_Read,
    Mem7zLookInStream_Seek
};

void InitMem7zLookInStream(CMem7zLookInStream* stream, const Byte* data, size_t dataSize) {
    memcpy(&stream->vtbl, &Mem7zLookInStreamVtbl, sizeof(ILookInStream));
    stream->data = data;
    stream->size = dataSize;
    stream->position = 0;
}
