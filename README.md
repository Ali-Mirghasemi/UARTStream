# UART Stream Library

This library implement Stream over UART based on STM32CubeMX HAL Driver

## How to use
Following instructions shows how to use it for `USART1`

1. Configure UART in CubeMX and generate the code
    > UART Must be in IT or DMA mode

2. With CubeMX `USE_HAL_UART_REGISTER_CALLBACKS=1`, create an object and
   initialize it **after** `MX_USARTx_UART_Init()`:

    ```c
    static uint8_t    RxBuffer[128];
    static uint8_t    TxBuffer[128];
    static UARTStream Uart;

    if (!UARTStream_init(&Uart, &huart1,
                         RxBuffer, sizeof(RxBuffer),
                         TxBuffer, sizeof(TxBuffer))) {
        /* UART callback registration failed; do not start communication. */
    }
    ```

   `UARTStream_init()` automatically registers `TX_COMPLETE`, `RX_COMPLETE`
   and `ERROR` handlers. No user weak callback or manual registration is needed.
   `UARTStream_initIdle()` also registers the receive-to-idle event callback.
   One stream is supported by default; set `UARTSTREAM_MAX_INSTANCES` before
   including the header to reserve more static slots for other UARTs.

3. Call `UARTStream_handle(&Uart)` in the foreground loop. The library reports
   UART errors from interrupts using a one-byte flag, and recovers outside
   interrupt context. TX/RX completion uses the existing Stream handlers.

4. If CubeMX callback registration is disabled, implement the HAL weak callbacks
   yourself and forward events to `UARTStream_txHandle()`,
   `UARTStream_rxHandle()`, and `UARTStream_errorHandle()` as before.

**Runtime:** no heap, no polling waits, no extra UART adapter, and no new IRQ
handlers. HAL must have initialized the UART before `UARTStream_init()`.

# References
[Stream Library](https://github.com/Ali-Mirghasemi/Stream)



