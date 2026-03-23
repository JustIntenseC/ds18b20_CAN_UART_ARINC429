#include "uart.h"


#define MAX_PACKET_SIZE 256
UART_AbstractionTypeDef huart1;
uint8_t packet_received = 0;
uint8_t packet_index = 0;
char packet_buffer[MAX_PACKET_SIZE] = "";
volatile uint8_t processCommandFlag = 0;



void  USB_UART_Init(void){
    RCU->UARTCFG[1].UARTCFG_bit.DIVEN = 0;
    RCU->UARTCFG[1].UARTCFG_bit.CLKSEL = RCU_UARTCFG_UARTCFG_CLKSEL_PLLCLK; 
    RCU->UARTCFG[1].UARTCFG_bit.RSTDIS = 1;
    RCU->UARTCFG[1].UARTCFG_bit.CLKEN = 1;
    huart1.Instance = UART1;
    huart1.BaudRate = 115200U;
    huart1.WLEN = UART_LCRH_WLEN_8bit;
    huart1.STOPBIT = 0U;
    huart1.RXENABLE = 1U;
    huart1.TXENABLE = 1U;
    UART1->IMSC_bit.RXIM = 1;
    NVIC_SetPriority(UART1_RX_IRQn, 1);
    NVIC_EnableIRQ(UART1_RX_IRQn);
    User_UartInit(&huart1);
    // printf("K1921VK035 microcontroller - UART1 READY\n");
}
void User_UartInit(UART_AbstractionTypeDef *huart){
    uint32_t integerDivider = 100000000U / (16 * huart->BaudRate);
    huart->Instance->IBRD = integerDivider;
    huart->Instance->FBRD = (uint32_t)((100000000U/ (16.0f * huart ->BaudRate)-integerDivider) * 64.0f + 0.5);
    huart->Instance->LCRH_bit.WLEN = huart->WLEN;
    huart->Instance->CR_bit.RXE = huart->RXENABLE;
    huart->Instance->CR_bit.TXE = huart->TXENABLE;
    huart->Instance->CR_bit.UARTEN = 0x1;
}

void USB_UART_SendByte(uint8_t *byte){
    uint32_t timeout = 1000000;
    while ((UART1->FR_bit.BUSY == 1) && timeout--) {};
    if (timeout == 0) Error_Handler(); 
    UART1->DR = *byte;
}

void USB_UART_SendMessage(uint8_t *msg, uint16_t length){
    for (int i = 0; i < length; i++) {
        USB_UART_SendByte(&(msg[i]));
        while(UART1->FR_bit.BUSY == 1){};
    }
}

void UART1_RX_IRQHandler(void) {
    uint8_t byte = UART1->DR & 0xFF;
    if (packet_index < MAX_PACKET_SIZE) {
        packet_buffer[packet_index] = byte;
        packet_index++;
        if (byte == '\n') {
            packet_received =1;
            packet_index = 0;
            processCommandFlag = 1;
        }
    } else {
        packet_index = 0;
    }
    UART1->ICR_bit.RXIC = 1;
}

void printUART(uint8_t *msg){
    USB_UART_SendMessage(msg, strlen((char*)msg));
}


