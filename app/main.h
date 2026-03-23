#ifndef MAIN_H
#define MAIN_H
#include <stdio.h>
#include <string.h>
#include "K1921VK035.h"





typedef enum{  // state
    STATE_IDLE,
    STATE_MEASURING,
    STATE_WAIT_MEASURING,
    STATE_READING,
    STATE_READ_DONE,
    STATE_ERROR
}   STATE;


typedef enum{
    MODE_AUTO,
    MODE_MANUAL,
    MODE_IDLE,
    MODE_ERROR
} STATE_MODE;

typedef enum{
    UART_MSG,
    CAN_MSG,
    ARINC429_MSG,
} TYPE_MSG;


typedef struct{
    STATE CurrentState;
    uint8_t scratchpad [9];
    uint8_t rom_address[8];
    float last_temperature;
    uint8_t object_RX;
    uint8_t object_TX;
    TYPE_MSG type_msg;


} DS18B20_State;

typedef struct 
{
    STATE_MODE CurrentMode;
} DS18B20_Mode;



void HandleEventDS18B20(DS18B20_State *ds18b20State);
void HandleModeDS18B20(DS18B20_Mode *currentMode, DS18B20_State *ds18b20State);
void selectMode(STATE_MODE *currentMode);


void ProcessCommand(char *msg, DS18B20_State* ds18b20, DS18B20_Mode* ds18b20Mode);

static void Timer0_Init(void);
void Timer_Delay_Ticks(uint32_t ticks); // TMR0 block for OneWire us
void Timer_Delay_Ticks_NoneBLocking(uint32_t ticks);
static void Timer1_Init(void);

uint8_t timeToMeasureDS18B20(void);


static void GPIO_Init(void);


void Error_Handler(void);





















#endif 






