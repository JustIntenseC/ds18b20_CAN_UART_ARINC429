#ifndef CAN_H
#define CAN_H
#include "K1921VK035.h"
#include "stdint.h"
#include <stdio.h>
#include "stdbool.h"
#include "ds18b20.h"
#define QUEUE_BUFFER_SIZE 32 
#define OBJECT_ONE_CAN_RX 1
#define OBJECT_TWO_CAN_TX 2

typedef enum{
    CAN_OPERATION_TX,
    CAN_OPERATION_RX, 
    CAN_OPERATION_TXRX
}   CAN_Operation_TypeDef;
typedef enum{
    CAN_MESSAGE_REMOTE,
    CAN_MESSAGE_COMMON
}   CAN_Message_TypeDef;

typedef struct{
    uint32_t ID;
    uint8_t DLC;
    uint8_t data[8];
    uint8_t flags;
} CAN_Message;

typedef struct{
    CAN_Message buffer [QUEUE_BUFFER_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t count;
} CAN_Queue_t;

typedef struct{
    uint32_t id;
    void (*handler)(CAN_Message*); 
} CAN_Command;


void CAN_Init(void);
void CAN_Object_Location(uint32_t obj_first_num, uint32_t obj_last_num,
                         uint32_t list_num);
void CAN_Object_Config(uint32_t obj_num, CAN_Operation_TypeDef op_type, CAN_Message_TypeDef msg_type);   
void CAN_Object_Transmit(uint32_t obj_num);
void CAN_Object_Receive(uint32_t obj_num);
void CAN_CompareData(uint32_t tx_obj_num, uint32_t rx_obj_num);
void CAN_ReadData(uint32_t obj_num);
uint8_t CAN_Test(void);
void CAN_Check_Errors(uint8_t nodeNum);
void CAN_Handler(DS18B20_State *ds18b20State);
static bool CAN_Queue_Put(CAN_Message *msg);
static bool CAN_Queue_Get(CAN_Message *msg);
// void CAN_R
#endif