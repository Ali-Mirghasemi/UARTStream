#include "UARTStream.h"

/** @brief Keep only a small fixed registry, with no heap allocation. */
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1U)
static UARTStream* UARTStream_Registry[UARTSTREAM_MAX_INSTANCES];

/**
 * @brief Find the stream belonging to an STM32 HAL UART callback.
 * @param huart HAL UART handle passed by the IRQ service.
 * @return Registered stream, or NULL when the peripheral is not managed.
 */
static UARTStream* UARTStream_find(UART_HandleTypeDef* huart) {
    uint8_t i;
    if (huart == NULL) { return NULL; }
    switch ((uint32_t)(uintptr_t)huart->Instance) {
#if defined(USART1_BASE)
        case USART1_BASE:
#endif
#if defined(USART2_BASE)
        case USART2_BASE:
#endif
#if defined(USART3_BASE)
        case USART3_BASE:
#endif
#if defined(UART4_BASE)
        case UART4_BASE:
#endif
#if defined(UART5_BASE)
        case UART5_BASE:
#endif
#if defined(USART6_BASE)
        case USART6_BASE:
#endif
            for (i = 0U; i < UARTSTREAM_MAX_INSTANCES; i++) {
                UARTStream* stream = UARTStream_Registry[i];
                if (stream != NULL && stream->HUART == huart) { return stream; }
            }
            break;
        default:
            break;
    }
    return NULL;
}

/** @brief Process an asynchronous transmission completion. */
static void UARTStream_onTx(UART_HandleTypeDef* huart) {
    UARTStream* stream = UARTStream_find(huart);
    if (stream != NULL) { UARTStream_txHandle(stream); }
}

/** @brief Process a received stream frame. */
static void UARTStream_onRx(UART_HandleTypeDef* huart) {
    UARTStream* stream = UARTStream_find(huart);
    if (stream != NULL) { UARTStream_rxHandle(stream); }
}

/** @brief Defer recovery from a UART error to the foreground. */
static void UARTStream_onError(UART_HandleTypeDef* huart) {
    UARTStream* stream = UARTStream_find(huart);
    if (stream != NULL) { stream->ErrorPending = 1U; }
}

#if UARTSTREAM_SUPPORT_IDLE
/** @brief Pass idle receive events to the configured stream. */
static void UARTStream_onRxEvent(UART_HandleTypeDef* huart, uint16_t length) {
    UARTStream* stream = UARTStream_find(huart);
    if (stream != NULL) { UARTStream_rxHandleIdle(stream, (Stream_LenType)length); }
}
#endif

/**
 * @brief Install the three UART callbacks after HAL_UART_Init has completed.
 * @return One for a complete registration; zero for an unavailable slot or HAL error.
 */
static uint8_t UARTStream_autoRegister(UARTStream* stream) {
    uint8_t i;
    uint8_t freeSlot = UARTSTREAM_MAX_INSTANCES;
    if (stream == NULL || stream->HUART == NULL) { return 0U; }
    for (i = 0U; i < UARTSTREAM_MAX_INSTANCES; i++) {
        if (UARTStream_Registry[i] != NULL &&
            UARTStream_Registry[i]->HUART == stream->HUART &&
            UARTStream_Registry[i] != stream) { return 0U; }
        if (UARTStream_Registry[i] == stream) { freeSlot = i; break; }
        if (UARTStream_Registry[i] == NULL && freeSlot == UARTSTREAM_MAX_INSTANCES) {
            freeSlot = i;
        }
    }
    if (freeSlot == UARTSTREAM_MAX_INSTANCES) { return 0U; }
    UARTStream_Registry[freeSlot] = stream;
    if (HAL_UART_RegisterCallback(stream->HUART, HAL_UART_TX_COMPLETE_CB_ID,
                                  UARTStream_onTx) != HAL_OK ||
        HAL_UART_RegisterCallback(stream->HUART, HAL_UART_RX_COMPLETE_CB_ID,
                                  UARTStream_onRx) != HAL_OK ||
        HAL_UART_RegisterCallback(stream->HUART, HAL_UART_ERROR_CB_ID,
                                  UARTStream_onError) != HAL_OK) {
        (void)HAL_UART_UnRegisterCallback(stream->HUART, HAL_UART_TX_COMPLETE_CB_ID);
        (void)HAL_UART_UnRegisterCallback(stream->HUART, HAL_UART_RX_COMPLETE_CB_ID);
        (void)HAL_UART_UnRegisterCallback(stream->HUART, HAL_UART_ERROR_CB_ID);
        UARTStream_Registry[freeSlot] = NULL;
        return 0U;
    }
    return 1U;
}
#endif

/**
 * @brief Initialize UARTStream object
 * 
 * @param stream 
 * @param huart 
 * @param rxBuff 
 * @param rxBuffSize 
 * @param txBuff 
 * @param txBuffSize 
 */
uint8_t UARTStream_init(
	UARTStream* stream, UART_HandleTypeDef* huart, 
	uint8_t* rxBuff, Stream_LenType rxBuffSize, 
	uint8_t* txBuff, Stream_LenType txBuffSize
) {
    stream->Args = NULL;
    stream->HUART = huart;
    stream->ErrorPending = 0U;

    IStream_init(&stream->Input, UARTStream_receive, rxBuff, rxBuffSize);
    IStream_setCheckReceive(&stream->Input, UARTStream_checkReceive);
    IStream_setArgs(&stream->Input, stream);

    OStream_init(&stream->Output, UARTStream_transmit, txBuff, txBuffSize);
    OStream_setCheckTransmit(&stream->Output, UARTStream_checkTransmit);
    OStream_setArgs(&stream->Output, stream);
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1U)
    return UARTStream_autoRegister(stream);
#else
    return 1U;
#endif
}
/**
 * @brief Use HAL per-instance callback pointers instead of overriding weak symbols.
 * @return One when all selected callbacks have been installed.
 */
uint8_t UARTStream_registerCallbacks(UARTStream* stream,
                                     UARTStream_CallbackFn onTx,
                                     UARTStream_CallbackFn onRx,
                                     UARTStream_CallbackFn onError) {
#if USE_HAL_UART_REGISTER_CALLBACKS
    if (stream == NULL || stream->HUART == NULL) { return 0U; }
    if (onTx != NULL &&
        HAL_UART_RegisterCallback(stream->HUART, HAL_UART_TX_COMPLETE_CB_ID,
                                  onTx) != HAL_OK) { return 0U; }
    if (onRx != NULL &&
        HAL_UART_RegisterCallback(stream->HUART, HAL_UART_RX_COMPLETE_CB_ID,
                                  onRx) != HAL_OK) { return 0U; }
    if (onError != NULL &&
        HAL_UART_RegisterCallback(stream->HUART, HAL_UART_ERROR_CB_ID,
                                  onError) != HAL_OK) { return 0U; }
    return 1U;
#else
    (void)stream;
    (void)onTx;
    (void)onRx;
    (void)onError;
    return 0U;
#endif
}

/**
 * @brief Handle pending UART error recovery outside the interrupt context.
 * @param stream Initialized UARTStream instance.
 */
void UARTStream_handle(UARTStream* stream) {
    if (stream != NULL && stream->ErrorPending != 0U) {
        stream->ErrorPending = 0U;
        UARTStream_errorHandle(stream);
    }
}

/**
 * @brief Handle Receive Data for Input Stream, user must put it in 
 * HAL_UART_RxCpltCallback or other callbacks
 * 
 * @param stream 
 */
void UARTStream_rxHandle(UARTStream* stream) {
    IStream_handle(&stream->Input, IStream_incomingBytes(&stream->Input));
}
/**
 * @brief Handle Transmit Data for Output Stream, user must put it in 
 * HAL_UART_TxCpltCallback or other callbacks
 * 
 * @param stream 
 */
void UARTStream_txHandle(UARTStream* stream) {
    OStream_handle(&stream->Output, OStream_outgoingBytes(&stream->Output));
}
/**
 * @brief Handle Error for Stream, user must put it in HAL_UART_ErrorCallback
 * 
 * @param stream 
 */
void UARTStream_errorHandle(UARTStream* stream) {
    IStream_resetIO(&stream->Input);
    OStream_resetIO(&stream->Output);
    IStream_receive(&stream->Input);
}
#if UARTSTREAM_SUPPORT_IDLE
/**
 * @brief Initialize UARTStream object for Idle
 * 
 * @param stream 
 * @param huart 
 * @param rxBuff 
 * @param rxBuffSize 
 * @param txBuff 
 * @param txBuffSize 
 */
uint8_t UARTStream_initIdle(
    UARTStream* stream, UART_HandleTypeDef* huart, 
    uint8_t* rxBuff, Stream_LenType rxBuffSize, 
    uint8_t* txBuff, Stream_LenType txBuffSize
) {
    if (!UARTStream_init(stream, huart, rxBuff, rxBuffSize, txBuff, txBuffSize)) {
        return 0U;
    }
    stream->Input.receive = UARTStream_receiveIdle;
#if (USE_HAL_UART_REGISTER_CALLBACKS == 1U)
    if (HAL_UART_RegisterRxEventCallback(huart, UARTStream_onRxEvent) != HAL_OK) {
        return 0U;
    }
#endif
    return 1U;
}
/**
 * @brief Handle Idle for Stream, user must put it in HAL_UART_RxIdleCallback
 * 
 * @param stream 
 * @param len 
 */
void UARTStream_rxHandleIdle(UARTStream* stream, Stream_LenType len) {
    IStream_handle(&stream->Input, len);
}
/**
 * @brief Stream receive function for Idle
 * 
 * @param stream 
 * @param buff 
 * @param len 
 */
Stream_Result UARTStream_receiveIdle(IStream* stream, uint8_t* buff, Stream_LenType len) {
    HAL_StatusTypeDef status;
    UARTStream* uartStream = (UARTStream*) IStream_getArgs(stream);
    if (uartStream->HUART->hdmarx) {
        status = HAL_UARTEx_ReceiveToIdle_DMA(uartStream->HUART, buff, len);
    }
    else {
        status = HAL_UARTEx_ReceiveToIdle_IT(uartStream->HUART, buff, len);
    }
    return status == HAL_OK ? Stream_Ok : (Stream_Result) (Stream_CustomError | status);
}
#endif
/**
 * @brief Check Received bytes in DMA or IT and add to stream
 * 
 * @param stream 
 * @return Stream_LenType 
 */
Stream_LenType UARTStream_checkReceive(IStream* stream) {
    UARTStream* uartStream = (UARTStream*) IStream_getArgs(stream);
    if (uartStream->HUART->hdmarx) {
        return IStream_incomingBytes(stream) - __HAL_DMA_GET_COUNTER(uartStream->HUART->hdmarx);
    }
    else {
        return IStream_incomingBytes(stream) - uartStream->HUART->RxXferCount;
    }
}
/**
 * @brief Check Transmitted bytes in DMA or IT and removed from stream 
 * 
 * @param stream 
 * @return Stream_LenType 
 */
Stream_LenType UARTStream_checkTransmit(OStream* stream) {
    UARTStream* uartStream = (UARTStream*) OStream_getArgs(stream);
    if (uartStream->HUART->hdmatx) {
        return OStream_outgoingBytes(stream) - __HAL_DMA_GET_COUNTER(uartStream->HUART->hdmatx);
    }
    else {
        return OStream_outgoingBytes(stream) - uartStream->HUART->TxXferCount;
    }
}
/**
 * @brief Stream receive function
 * 
 * @param stream 
 * @param buff 
 * @param len 
 */
Stream_Result UARTStream_receive(IStream* stream, uint8_t* buff, Stream_LenType len) {
    HAL_StatusTypeDef status;
    UARTStream* uartStream = (UARTStream*) IStream_getArgs(stream);
    if (uartStream->HUART->hdmarx) {
        status = HAL_UART_Receive_DMA(uartStream->HUART, buff, len);
    }
    else {
        status = HAL_UART_Receive_IT(uartStream->HUART, buff, len);
    }
    return status == HAL_OK ? Stream_Ok : (Stream_Result) (Stream_CustomError | status);
}
/**
 * @brief Stream transmit function
 * 
 * @param stream 
 * @param buff 
 * @param len 
 */
Stream_Result UARTStream_transmit(OStream* stream, uint8_t* buff, Stream_LenType len) {
    HAL_StatusTypeDef status;
    UARTStream* uartStream = (UARTStream*) OStream_getArgs(stream);
    if (uartStream->HUART->hdmatx) {
        status = HAL_UART_Transmit_DMA(uartStream->HUART, buff, len);
    }
    else {
        status = HAL_UART_Transmit_IT(uartStream->HUART, buff, len);
    }
    return status == HAL_OK ? Stream_Ok : (Stream_Result) (Stream_CustomError | status);
}
