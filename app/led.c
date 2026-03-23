#ifndef LED_H
#define LED_H


#include "led.h"
#include "can.h"

void Led_Pin_Init(void){

    RCU->HCLKCFG_bit.GPIOAEN = 1;
    RCU->HRSTCFG_bit.GPIOAEN = 1;
    GPIOA->OUTMODE_bit.PIN8 = GPIO_OUTMODE_PIN8_PP;
    GPIOA->DENSET_bit.PIN8 = 1;
    GPIOA->OUTENSET_bit.PIN8 = 1;
 

}

LED_STATUS Led_Pin_ON(void){

    GPIOA->DATAOUTSET_bit.PIN8 = 1;
    return STATUS_ON;

}
LED_STATUS Led_Pin_OFF(void){
    GPIOA->DATAOUTCLR_bit.PIN8 = 1;
    return STATUS_OFF;
}


void Led_Pin_Toggle(void){
    GPIOA->DATAOUTTGL_bit.PIN8  = 1;
}



LED_STATUS Led_Pin_ON_CAN(void){
    uint8_t object_tx = 1;
    uint8_t object_rx = 2;
    CANMSG->Msg[object_tx].MODATAL = 0x1 << CANMSG_Msg_MODATAL_DB0_Pos;
    CAN_Object_Transmit(object_tx);
    while(CANMSG->Msg[object_rx].MOSTAT_bit.NEWDAT == 0) {}
    if((CANMSG ->Msg[object_rx].MODATAL >> CANMSG_Msg_MODATAL_DB0_Pos) & 0x01){
        GPIOA->DATAOUTSET_bit.PIN8 =  STATUS_ON;
    }
    CANMSG->Msg[object_rx].MOCTR = CANMSG_Msg_MOCTR_RESNEWDAT_Msk;
    return STATUS_ON;
}
LED_STATUS Led_Pin_OFF_CAN(void){
    uint8_t object_tx = 1;
    uint8_t object_rx = 2;
    CANMSG->Msg[object_tx].MODATAL = 0x0 << CANMSG_Msg_MODATAL_DB0_Pos;
    CAN_Object_Transmit(object_tx);
    while(CANMSG->Msg[object_rx].MOSTAT_bit.NEWDAT == 0) {}
    if(!((CANMSG ->Msg[object_rx].MODATAL >> CANMSG_Msg_MODATAL_DB0_Pos) & 0x01)){
        GPIOA->DATAOUTCLR_bit.PIN8 =  1;
    }
    CANMSG->Msg[object_rx].MOCTR = CANMSG_Msg_MOCTR_RESNEWDAT_Msk;
    return STATUS_OFF;
}

#endif