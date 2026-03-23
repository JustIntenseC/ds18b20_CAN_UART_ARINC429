#include "arinc429.h"
void arinc429_init(arinc429_t *arinc429, GPIO_TypeDef *port, uint8_t pin_a, uint8_t pin_b){
    if(!arinc429 || !port) return;    
    arinc429->pins.port = GPIOB;
    arinc429->pins.PIN_A = 3;
    arinc429->pins.PIN_B = 4;    
    port->DENSET =  (1 << pin_a) | (1 << pin_b);
    port->OUTMODE =  (GPIO_OUTMODE_PIN3_PP << pin_a) | (GPIO_OUTMODE_PIN4_PP << pin_b);
    port->OUTENSET =  (1 << pin_a) | (1 << pin_b);  
    arinc429->speed = ARINC_SPEED_HIGH;

}
void arinc429_calc_parity(arinc429_t *arinc429){
    uint32_t *raw = &arinc429->tx.raw;
    *raw &= ~(1 << ARINC429_PARITY_Pos);
    uint8_t count = 0 ;
    for(uint32_t bit =  0; bit < (ARINCE429_PACKET_SIZE - 1 ); bit++){
        if(((*raw >> bit) & 0x1)) count ++;
    }
    arinc429->tx.bnr.PARITY = (count % 2 ==0) ? 1 : 0;
}
void arinc429_send_packet(arinc429_t *arinc429){
    if(!arinc429->tx.raw) return;
    arinc429_calc_parity(arinc429);
    uint32_t packet = arinc429->tx.raw;
    for(uint32_t bit = 0; bit < ARINCE429_PACKET_SIZE; bit++){
        arinc429_send_bit(arinc429, (packet >> bit) & 0x1);
    }
    Timer_Delay_Ticks(arinc429->speed == ARINC_SPEED_HIGH ? ARINC_BIT_TIME_10US*4 : ARINC_BIT_TIME_80US*4 ); // 100 kbit/s

}

void arinc429_send_bit(arinc429_t *arinc429, uint8_t bit){
    if(!arinc429) return;
    if(bit){
        arinc429->pins.port->DATAOUTSET_bit.PIN3 = 1;
        arinc429->pins.port->DATAOUTCLR_bit.PIN4 = 1;
    }
    else{
        arinc429->pins.port->DATAOUTSET_bit.PIN4 = 1;
        arinc429->pins.port->DATAOUTCLR_bit.PIN3 = 1;
    }
    Timer_Delay_Ticks(arinc429->speed == ARINC_SPEED_HIGH ? ARINC_BIT_TIME_10US : ARINC_BIT_TIME_80US); // 100 kbit/s
}

