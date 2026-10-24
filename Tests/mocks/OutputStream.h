/** @file OutputStream.h @brief Small Stream output mock for callback tests. */
#ifndef UARTSTREAM_TEST_OUTPUT_H
#define UARTSTREAM_TEST_OUTPUT_H
#include "InputStream.h"
typedef struct OStream OStream;
typedef Stream_Result (*OStream_TransmitFn)(OStream*, uint8_t*, Stream_LenType);
struct OStream {
    void* Args;
    OStream_TransmitFn transmit;
    Stream_LenType (*checkTransmit)(OStream*);
    uint16_t Pending;
};
void OStream_init(OStream*, OStream_TransmitFn, uint8_t*, Stream_LenType);
void OStream_setCheckTransmit(OStream*, Stream_LenType (*)(OStream*));
void OStream_setArgs(OStream*, void*);
void* OStream_getArgs(OStream*);
Stream_LenType OStream_outgoingBytes(OStream*);
void OStream_handle(OStream*, Stream_LenType);
void OStream_resetIO(OStream*);
#endif
