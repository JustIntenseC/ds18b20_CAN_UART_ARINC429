#ifndef UART_H
#define UART_H

#include "K1921VK035.h"
#include "stdint.h"
#include <stdio.h>
#define MAX_PACKET_SIZE 256
extern uint8_t packet_received;
extern uint8_t packet_index;
extern char packet_buffer[MAX_PACKET_SIZE];
extern volatile uint8_t processCommandFlag;
typedef struct{
    UART_TypeDef *Instance;
    uint32_t BaudRate;
    UART_LCRH_WLEN_Enum WLEN;
    uint32_t  STOPBIT;
    uint32_t RXENABLE;
    uint32_t TXENABLE;
}   UART_AbstractionTypeDef;
void USB_UART_Init(void);
void User_UartInit(UART_AbstractionTypeDef *huart);
void USB_UART_SendByte(uint8_t *byte);
void USB_UART_SendMessage(uint8_t *msg, uint16_t length);
void printUART(uint8_t *msg);
#endif