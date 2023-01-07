# UART Stream Library

This library implement Stream over UART based on STM32CubeMX HAL Driver

## How to use
Following instructions shows how to use it for `USART1`

1. Configure UART in CubeMX and generate the code
    > UART Must be in IT or DMA mode

2. Define your `UARTStream` variable and initialize in main after UART init

    **Global Area**
    ```C
    uint8_t rxBuf1[128];
    uint8_t txBuf1[128];
    UARTStream uart1;
    ```

    **Main Function**
    ```C
    UARTStream_init(
        &uart1, &huart1,
        rxBuf1, sizeof(rxBuf1),
        txBuf1, sizeof(txBuf1)
    );
    ```
3. Add `UARTStream` handle into callbacks
    ```C
    void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
        switch ((uint32_t) huart->Instance) {
            case USART1_BASE:
                UARTStream_rxHandle(&uart1);
                break;
            // you can add more UART here
        }
    }

    void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
        switch ((uint32_t) huart->Instance) {
            case USART1_BASE:
                UARTStream_txHandle(&uart1);
                break;
            // you can add more UART here
        }
    }
    ```
4. Start receive stream 
    ```C
    IStream_receive(&uart1.In);
    ```

# References
[Stream Library](https://github.com/Ali-Mirghasemi/Stream)



