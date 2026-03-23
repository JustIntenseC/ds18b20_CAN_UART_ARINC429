
#ifndef DS18B20_H
#define DS18B20_H

#include "uart.h"
#include "main.h"
#include "can.h"
#include "K1921VK035.h"
#define DS18B20_PIN 0
#define DS18B20_MEASURE_DELAY 750
#define DS18B20_READ_PIN() ((GPIOB->DATA >> DS18B20_PIN) & 0x1)
extern uint32_t msticks;
extern uint32_t start;

STATE DS18B20_Reset(void);
void DS18B20_WriteBit(uint8_t bit);
void DS18B20_WriteByte(uint8_t);
uint8_t DS18B20_ReadBit(void);
uint8_t DS18B20_ReadByte(void);
void DS18B20_ReadROM(uint8_t *rom);
uint8_t DS18B20_CRC8(const uint8_t *data, uint8_t len);
uint8_t timeToReadDS18B20(void);
void DS18B20_SendMessage(DS18B20_State *ds18b20State);
void DS18B20_SendMessage_CAN(DS18B20_State *ds18b20State);
void DS18B20_Convert(DS18B20_State *ds18b20State);

#endif