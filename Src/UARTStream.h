/**
 * @file UARTStream.h
 * @author Ali Mirghasemi (ali.mirghasemi1376@gmail.com)
 * @brief This library implement Stream library for STM32Fxxx Based on HAL
 * @version 0.1
 * @date 2023-01-07
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#ifndef _UART_STREAM_H_
#define _UART_STREAM_H_

#include "InputStream.h"
#include "OutputStream.h"


/******************************************************************************/
/*                                Configuration                               */
/******************************************************************************/
/**
 * @brief Enable/Disable UARTStream support for idle
*/
#define UARTSTREAM_SUPPORT_IDLE        1

/** @brief Maximum number of concurrently initialized UARTStream objects. */
#ifndef UARTSTREAM_MAX_INSTANCES
#define UARTSTREAM_MAX_INSTANCES       1U
#endif

/******************************************************************************/
#include "main.h"

/** @brief UART transport adapter without an additional stream allocation. */
typedef struct {
    void*               Args;    /**< Optional application context; initialized to NULL. */
    UART_HandleTypeDef* HUART;   /**< CubeMX-generated peripheral handle. */
    IStream             Input;   /**< Existing Stream input state. */
    OStream             Output;        /**< Existing Stream output state. */
    volatile uint8_t    ErrorPending;  /**< ISR error event consumed in foreground. */
} UARTStream;

/**
 * @brief Initialize a UARTStream and automatically register HAL callbacks.
 * @return One on success, zero if a callback could not be registered.
 * @note With USE_HAL_UART_REGISTER_CALLBACKS=0, legacy weak callbacks remain
 *       the application's responsibility.
 */
uint8_t UARTStream_init(
	UARTStream* stream, UART_HandleTypeDef* huart, 
	uint8_t* rxBuff, Stream_LenType rxBuffSize, 
	uint8_t* txBuff, Stream_LenType txBuffSize
);

/**
 * @brief Optionally replace automatically installed callbacks with custom ones.
 * @param stream Initialized UARTStream instance.
 * @param onTx Transmit-complete callback, or NULL to retain the current handler.
 * @param onRx Receive-complete callback, or NULL to retain the current handler.
 * @param onError UART error callback, or NULL to retain the current handler.
 * @return One on success, zero on failure.
 * @note Normal users never call this; UARTStream_init registers all callbacks.
 * @note Custom overrides are responsible for calling the stream handlers.
 */
/** @brief Application-provided HAL-compatible UART event callback pointer. */
typedef void (*UARTStream_CallbackFn)(UART_HandleTypeDef* huart);

uint8_t UARTStream_registerCallbacks(UARTStream* stream,
                                     UARTStream_CallbackFn onTx,
                                     UARTStream_CallbackFn onRx,
                                     UARTStream_CallbackFn onError);

/** @brief Process deferred UART errors from foreground code. */
void UARTStream_handle(UARTStream* stream);

void UARTStream_rxHandle(UARTStream* stream);
void UARTStream_txHandle(UARTStream* stream);
void UARTStream_errorHandle(UARTStream* stream);

Stream_LenType UARTStream_checkReceive(IStream* stream);
Stream_LenType UARTStream_checkTransmit(OStream* stream);
Stream_Result UARTStream_receive(IStream* stream, uint8_t* buff, Stream_LenType len);
Stream_Result UARTStream_transmit(OStream* stream, uint8_t* buff, Stream_LenType len);

#if UARTSTREAM_SUPPORT_IDLE
	uint8_t UARTStream_initIdle(
		UARTStream* stream, UART_HandleTypeDef* huart, 
		uint8_t* rxBuff, Stream_LenType rxBuffSize, 
		uint8_t* txBuff, Stream_LenType txBuffSize
	);
	void UARTStream_rxHandleIdle(UARTStream* stream, Stream_LenType len);
	Stream_Result UARTStream_receiveIdle(IStream* stream, uint8_t* buff, Stream_LenType len);
#endif

#endif // _UART_STREAM_H_

