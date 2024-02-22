#include <7z/7zTypes.h>
#ifdef __cplusplus__
extern "C" {
#endif
typedef struct {
    ILookInStream vtbl;
    UInt64 position;
    UInt64 size;
    const Byte* data;
} CMem7zLookInStream;

// º¯ÊýÉùÃ÷
SRes Mem7zLookInStream_Look(ILookInStreamPtr p, const void** buf, size_t* size);
SRes Mem7zLookInStream_Skip(ILookInStreamPtr p, size_t offset);
SRes Mem7zLookInStream_Read(ILookInStreamPtr p, void* buf, size_t* size);
SRes Mem7zLookInStream_Seek(ILookInStreamPtr p, Int64* pos, ESzSeek origin);

void InitMem7zLookInStream(CMem7zLookInStream* stream, const Byte* data, size_t dataSize);

#ifdef __cplusplus__
}
#endif