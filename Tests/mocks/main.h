/** @file main.h @brief Small HAL mock for UARTStream callback tests. */
#ifndef UARTSTREAM_TEST_MAIN_H
#define UARTSTREAM_TEST_MAIN_H
#include <stdint.h>
#include <stddef.h>
#define USE_HAL_UART_REGISTER_CALLBACKS 1U
#define USART1_BASE 0x40013800UL
#define USART2_BASE 0x40004400UL
#define USART3_BASE 0x40004800UL
typedef struct { uint16_t Counter; } DMA_HandleTypeDef;
typedef struct __UART_HandleTypeDef UART_HandleTypeDef;
typedef void (*UART_Test_Callback)(UART_HandleTypeDef*);
struct __UART_HandleTypeDef {
    void*              Instance;
    DMA_HandleTypeDef* hdmarx;
    DMA_HandleTypeDef* hdmatx;
    UART_Test_Callback Callbacks[5];
    void (*RxEvent)(UART_HandleTypeDef*, uint16_t);
    uint16_t           RxXferCount;
    uint16_t           TxXferCount;
};
typedef enum { HAL_OK = 0, HAL_ERROR = 1 } HAL_StatusTypeDef;
typedef enum {
    HAL_UART_TX_COMPLETE_CB_ID = 1,
    HAL_UART_RX_COMPLETE_CB_ID = 3,
    HAL_UART_ERROR_CB_ID = 4
} HAL_UART_CallbackIDTypeDef;
#define __HAL_DMA_GET_COUNTER(DMA) ((DMA)->Counter)
HAL_StatusTypeDef HAL_UART_RegisterCallback(UART_HandleTypeDef*, HAL_UART_CallbackIDTypeDef, UART_Test_Callback);
HAL_StatusTypeDef HAL_UART_UnRegisterCallback(UART_HandleTypeDef*, HAL_UART_CallbackIDTypeDef);
HAL_StatusTypeDef HAL_UART_RegisterRxEventCallback(UART_HandleTypeDef*, void (*)(UART_HandleTypeDef*, uint16_t));
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef*, uint8_t*, uint16_t);
HAL_StatusTypeDef HAL_UART_Receive_IT(UART_HandleTypeDef*, uint8_t*, uint16_t);
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef*, uint8_t*, uint16_t);
HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef*, uint8_t*, uint16_t);
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_DMA(UART_HandleTypeDef*, uint8_t*, uint16_t);
HAL_StatusTypeDef HAL_UARTEx_ReceiveToIdle_IT(UART_HandleTypeDef*, uint8_t*, uint16_t);
#endif
