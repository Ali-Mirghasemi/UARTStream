/** @file InputStream.h @brief Small Stream input mock for callback tests. */
#ifndef UARTSTREAM_TEST_INPUT_H
#define UARTSTREAM_TEST_INPUT_H
#include <stdint.h>
typedef uint16_t Stream_LenType;
typedef enum { Stream_Ok = 0, Stream_CustomError = 0x80 } Stream_Result;
typedef struct IStream IStream;
typedef Stream_Result (*IStream_ReceiveFn)(IStream*, uint8_t*, Stream_LenType);
struct IStream {
    void* Args;
    IStream_ReceiveFn receive;
    Stream_LenType (*checkReceive)(IStream*);
    uint16_t Pending;
};
void IStream_init(IStream*, IStream_ReceiveFn, uint8_t*, Stream_LenType);
void IStream_setCheckReceive(IStream*, Stream_LenType (*)(IStream*));
void IStream_setArgs(IStream*, void*);
void* IStream_getArgs(IStream*);
Stream_LenType IStream_incomingBytes(IStream*);
void IStream_handle(IStream*, Stream_LenType);
void IStream_resetIO(IStream*);
Stream_Result IStream_receive(IStream*);
#endif
