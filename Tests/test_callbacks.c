/**
 * @file test_callbacks.c
 * @brief UARTStream auto-registration and deferred-error host test.
 */
#include "UARTStream.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

/** @brief Fault injection for HAL callback-registration tests. */
static uint8_t HAL_FAIL_CALLBACK;
/** @brief Counters make completion and recovery effects observable. */
static uint16_t TX_HANDLED;
static uint16_t RX_HANDLED;
static uint16_t RX_RESET;
static uint16_t TX_RESET;
static uint16_t RX_RESTART;
static uint16_t IDLE_HANDLED;

/** @brief Register a test callback, or inject an intentional error. */
HAL_StatusTypeDef HAL_UART_RegisterCallback(UART_HandleTypeDef* HU,
                                             HAL_UART_CallbackIDTypeDef ID,
                                             UART_Test_Callback FN) {
    if ((uint8_t)ID == HAL_FAIL_CALLBACK) { return HAL_ERROR; }
    HU->Callbacks[ID] = FN;
    return HAL_OK;
}
/** @brief Restore the default (unset) callback. */
HAL_StatusTypeDef HAL_UART_UnRegisterCallback(UART_HandleTypeDef* HU,
                                               HAL_UART_CallbackIDTypeDef ID) {
    HU->Callbacks[ID] = NULL;
    return HAL_OK;
}
/** @brief Register the optional receive-to-idle event callback. */
HAL_StatusTypeDef HAL_UART_RegisterRxEventCallback(UART_HandleTypeDef* HU,
    void (*FN)(UART_HandleTypeDef*, uint16_t)) {
    HU->RxEvent = FN;
    return HAL_OK;
}
/** @brief Mock HAL async calls without transmitting physical bytes. */
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef* HU, uint8_t* B, uint16_t N) {
    (void)HU; (void)B; (void)N; return HAL_OK;
}
/** @brief Mock HAL async calls without transmitting physical bytes. */
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef* HU, uint8_t* B, uint16_t N) {
    (void)HU; (void)B; (void)N; return HAL_OK;
}
/** @brief Mock HAL async calls without transmitting physical bytes. */
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef* HU, uint8_t* B, uint16_t N) {
    (void)HU; (void)B; (void)N; return HAL_OK;
}
/** @brief Mock HAL async calls without transmitting physical bytes. */
HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef* HU, uint8_t* B, uint16_t N) {
    (void)HU; (void)B; (void)N; return HAL_OK;
}
/** @brief Mock the HAL async idle-DMA API. */
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_DMA(UART_HandleTypeDef* HU, uint8_t* B, uint16_t N) {
    (void)HU; (void)B; (void)N; return HAL_OK;
}
/** @brief Mock the HAL async idle-IT API. */
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_IT(UART_HandleTypeDef* HU, uint8_t* B, uint16_t N) {
    (void)HU; (void)B; (void)N; return HAL_OK;
}
/** @brief Initialize a fake input Stream. */
void IStream_init(IStream* S, IStream_ReceiveFn FN, uint8_t* B, Stream_LenType N) {
    (void)B; (void)N; S->receive = FN; S->Pending = 0;
}
/** @brief Assign a receive progress callback. */
void IStream_setCheckReceive(IStream* S, Stream_LenType (*FN)(IStream*)) { S->checkReceive = FN; }
/** @brief Assign the parent stream context. */
void IStream_setArgs(IStream* S, void* A) { S->Args = A; }
/** @brief Retrieve the parent stream context. */
void* IStream_getArgs(IStream* S) { return S->Args; }
/** @brief Return fake pending receive bytes. */
Stream_LenType IStream_incomingBytes(IStream* S) { return S->Pending; }
/** @brief Record an RX completion. */
void IStream_handle(IStream* S, Stream_LenType N) { (void)S; RX_HANDLED++; IDLE_HANDLED += N; }
/** @brief Record an RX reset. */
void IStream_resetIO(IStream* S) { (void)S; RX_RESET++; }
/** @brief Record an RX restart. */
Stream_Result IStream_receive(IStream* S) { (void)S; RX_RESTART++; return Stream_Ok; }
/** @brief Initialize a fake output Stream. */
void OStream_init(OStream* S, OStream_TransmitFn FN, uint8_t* B, Stream_LenType N) {
    (void)B; (void)N; S->transmit = FN; S->Pending = 0;
}
/** @brief Assign a transmit progress callback. */
void OStream_setCheckTransmit(OStream* S, Stream_LenType (*FN)(OStream*)) { S->checkTransmit = FN; }
/** @brief Assign the parent stream context. */
void OStream_setArgs(OStream* S, void* A) { S->Args = A; }
/** @brief Retrieve the parent stream context. */
void* OStream_getArgs(OStream* S) { return S->Args; }
/** @brief Return fake pending transmitted bytes. */
Stream_LenType OStream_outgoingBytes(OStream* S) { return S->Pending; }
/** @brief Record a TX completion. */
void OStream_handle(OStream* S, Stream_LenType N) { (void)S; (void)N; TX_HANDLED++; }
/** @brief Record a TX reset. */
void OStream_resetIO(OStream* S) { (void)S; TX_RESET++; }

/** @brief Exercise automatic registration, dispatch, and deferred recovery. */
int main(void) {
    UARTStream STREAM = {0};
    UARTStream OTHER = {0};
    UART_HandleTypeDef HU = {0};
    UART_HandleTypeDef SECOND = {0};
    uint8_t RX[8] = {0};
    uint8_t TX[8] = {0};
    HU.Instance = (void*)(uintptr_t)USART1_BASE;
    SECOND.Instance = (void*)(uintptr_t)USART2_BASE;
    HAL_FAIL_CALLBACK = (uint8_t)HAL_UART_RX_COMPLETE_CB_ID;
    assert(!UARTStream_init(&STREAM, &HU, RX, sizeof(RX), TX, sizeof(TX)));
    assert(HU.Callbacks[HAL_UART_TX_COMPLETE_CB_ID] == NULL);
    HAL_FAIL_CALLBACK = 0U;
    assert(UARTStream_init(&STREAM, &HU, RX, sizeof(RX), TX, sizeof(TX)));
    assert(HU.Callbacks[HAL_UART_TX_COMPLETE_CB_ID] != NULL);
    assert(HU.Callbacks[HAL_UART_RX_COMPLETE_CB_ID] != NULL);
    assert(HU.Callbacks[HAL_UART_ERROR_CB_ID] != NULL);
    assert(!UARTStream_init(&OTHER, &SECOND, RX, sizeof(RX), TX, sizeof(TX)));
    HU.Callbacks[HAL_UART_TX_COMPLETE_CB_ID](&HU);
    HU.Callbacks[HAL_UART_RX_COMPLETE_CB_ID](&HU);
    assert(TX_HANDLED == 1U && RX_HANDLED == 1U);
    HU.Callbacks[HAL_UART_ERROR_CB_ID](&HU);
    assert(STREAM.ErrorPending == 1U);
    assert(RX_RESET == 0U && TX_RESET == 0U);
    UARTStream_handle(&STREAM);
    assert(RX_RESET == 1U && TX_RESET == 1U && RX_RESTART == 1U);
    UARTStream_handle(&STREAM);
    assert(RX_RESET == 1U);
    assert(UARTStream_initIdle(&STREAM, &HU, RX, sizeof(RX), TX, sizeof(TX)));
    assert(HU.RxEvent != NULL);
    HU.RxEvent(&HU, 4U);
    assert(IDLE_HANDLED == 4U);
    puts("UARTStream callback test passed");
    return 0;
}
