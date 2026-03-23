#include "button.h"


void Button_Init(void){

    RCU->HCLKCFG_bit.GPIOAEN = 1;
    RCU->HRSTCFG_bit.GPIOAEN = 1;
    GPIOA->DENSET_bit.PIN7 = 1;
    GPIOA->INTTYPESET_bit.PIN7 = 1; 
    GPIOA->INTPOLSET_bit.PIN7 = 1;
    GPIOA->INTENSET_bit.PIN7 = 1;
    NVIC_EnableIRQ(GPIOA_IRQn);

}