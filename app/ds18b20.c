




#include "ds18b20.h"


uint32_t msticks = 0;
uint32_t start = 0 ;

STATE DS18B20_Reset(void){
    uint32_t timeout = 100000;
    GPIOB->DATAOUTCLR_bit.PIN0 = 1;
    Timer_Delay_Ticks(480);
    GPIOB->DATAOUTSET_bit.PIN0 = 1;
    while(( DS18B20_READ_PIN() == 1) && timeout){
          if(--timeout == 0){ printf("NOT RESPONSE DS18B20\n"); return STATE_ERROR; 
        };
    };
    while(( DS18B20_READ_PIN() == 0) && timeout){
           if(--timeout == 0){ printf("NOT RESPONSE DS18B20\n"); return STATE_ERROR; 
        };    
    };
    if(DS18B20_READ_PIN() == 1){
        Timer_Delay_Ticks(300);
    }
    else{
        return STATE_ERROR;
    }
}


void DS18B20_WriteBit(uint8_t bit){
    GPIOB->DATAOUTCLR_bit.PIN0 = 1; 
    Timer_Delay_Ticks( bit ? 1 : 60);
    GPIOB->DATAOUTSET_bit.PIN0 = 1;
    Timer_Delay_Ticks(60); 

}

void DS18B20_WriteByte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        DS18B20_WriteBit(byte & 0x01);
        byte = byte >> 1;
    }
}

uint8_t DS18B20_ReadBit(void){
    uint8_t bit;
    GPIOB->DATAOUTCLR_bit.PIN0 = 1;
    Timer_Delay_Ticks(10);
    GPIOB->DATAOUTSET_bit.PIN0 = 1;
    Timer_Delay_Ticks(15);
    bit  = DS18B20_READ_PIN();
    Timer_Delay_Ticks(60);
    return bit;
}

uint8_t DS18B20_ReadByte(void) {
    uint8_t byte = 0x00;
    for (uint8_t i = 0; i < 8; i++) {
        byte |= (DS18B20_ReadBit() << i);
    }
    return byte;
}
void DS18B20_ReadROM(uint8_t *rom){
    DS18B20_Reset();
    DS18B20_Reset();
    DS18B20_WriteByte(0x33);
    for(int i=0;i<8;i++){
        rom[i] = DS18B20_ReadByte();
    }
    
}

uint8_t DS18B20_CRC8(const uint8_t *data, uint8_t len){
    uint8_t crc = 0;
    for(int i=0;i<len;i++){
        crc ^=  data[i];
        for(int j=0;j<8;j++){
            if((crc & 0x01) == 0){
                crc >>=1;
            }
            else if((crc & 0x01) == 1){
                crc >>=1; crc ^= 0x8C;
            }
        }
    }
    return crc;
}

uint8_t timeToReadDS18B20(void){
    if(msticks - start >= DS18B20_MEASURE_DELAY){
        start = msticks;
        return 1;
    }
    else{
        return 0;
    }
}

void DS18B20_SendMessage(DS18B20_State *ds18b20State){
    DS18B20_Convert(ds18b20State);
    printf("Temperature (UART_MSG): %.2f C\n", ds18b20State->last_temperature);
    ds18b20State->CurrentState = STATE_IDLE;
}

void DS18B20_SendMessage_CAN(DS18B20_State *ds18b20State){
    CANMSG->Msg[ds18b20State->object_TX].MODATAL = (ds18b20State->scratchpad[0] << CANMSG_Msg_MODATAL_DB0_Pos) | 
                                                (ds18b20State->scratchpad[1] << CANMSG_Msg_MODATAL_DB1_Pos);
    CAN_Object_Transmit(ds18b20State->object_TX); 
    DS18B20_Convert(ds18b20State);
    printf("Temperature (CAN_MSG): %.2f C\n", ds18b20State->last_temperature);
    ds18b20State->CurrentState = STATE_IDLE;
}

void DS18B20_Convert(DS18B20_State *ds18b20State){
    int16_t temperature_raw = 0; float temperature = 0;
    switch (ds18b20State->type_msg)
    {
    case UART_MSG:
        temperature_raw = (ds18b20State->scratchpad[1] << 8) | ds18b20State->scratchpad[0];
        break;
    case CAN_MSG:
        temperature_raw = ((CANMSG->Msg[ds18b20State->object_TX].MODATAL >> CANMSG_Msg_MODATAL_DB1_Pos) & 0xFF) << 8|
                              ((CANMSG->Msg[ds18b20State->object_TX].MODATAL >> CANMSG_Msg_MODATAL_DB0_Pos) & 0xFF);
        break;
    case ARINC429_MSG:
        //
        break;
    default:
        break;
    }
    temperature = (float) temperature_raw / 16.0f;
    ds18b20State->last_temperature = temperature;
}

