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
#define UARTSTREAM_SUPPORT_IDLE		1

/******************************************************************************/
#include "main.h"

/** @brief UART transport adapter without an additional stream allocation. */
typedef struct {
    void*               Args;    /**< Optional application context; initialized to NULL. */
    UART_HandleTypeDef* HUART;   /**< CubeMX-generated peripheral handle. */
    IStream             Input;   /**< Existing Stream input state. */
    OStream             Output;  /**< Existing Stream output state. */
} UARTStream;

void UARTStream_init(
	UARTStream* stream, UART_HandleTypeDef* huart, 
	uint8_t* rxBuff, Stream_LenType rxBuffSize, 
	uint8_t* txBuff, Stream_LenType txBuffSize
);

/**
 * @brief Register application-owned HAL callbacks for an existing UARTStream.
 * @param stream Initialized UARTStream instance.
 * @param onTx Transmit-complete callback, or NULL to retain the HAL default.
 * @param onRx Receive-complete callback, or NULL to retain the HAL default.
 * @param onError UART error callback, or NULL to retain the HAL default.
 * @return One when all requested callbacks are registered, zero on failure.
 * @note Requires USE_HAL_UART_REGISTER_CALLBACKS=1 from STM32CubeMX.
 */
/** @brief Application-provided HAL-compatible UART event callback pointer. */
typedef void (*UARTStream_CallbackFn)(UART_HandleTypeDef* huart);

uint8_t UARTStream_registerCallbacks(UARTStream* stream,
                                     UARTStream_CallbackFn onTx,
                                     UARTStream_CallbackFn onRx,
                                     UARTStream_CallbackFn onError);

void UARTStream_rxHandle(UARTStream* stream);
void UARTStream_txHandle(UARTStream* stream);
void UARTStream_errorHandle(UARTStream* stream);

Stream_LenType UARTStream_checkReceive(IStream* stream);
Stream_LenType UARTStream_checkTransmit(OStream* stream);
Stream_Result UARTStream_receive(IStream* stream, uint8_t* buff, Stream_LenType len);
Stream_Result UARTStream_transmit(OStream* stream, uint8_t* buff, Stream_LenType len);

#if UARTSTREAM_SUPPORT_IDLE
	void UARTStream_initIdle(
		UARTStream* stream, UART_HandleTypeDef* huart, 
		uint8_t* rxBuff, Stream_LenType rxBuffSize, 
		uint8_t* txBuff, Stream_LenType txBuffSize
	);
	void UARTStream_rxHandleIdle(UARTStream* stream, Stream_LenType len);
	Stream_Result UARTStream_receiveIdle(IStream* stream, uint8_t* buff, Stream_LenType len);
#endif

#endif // _UART_STREAM_H_

