#ifndef ARINC429_H 
#define ARINC429_H
#include "K1921VK035.h"
#include "main.h"
#define ARINC429_PARITY_Pos 31
#define ARINCE429_PACKET_SIZE 32
#define ARINC_BIT_TIME_10US 10
#define ARINC_BIT_TIME_80US 80 
#define ARINC_SPEED_HIGH 100000
#define ARINC_SPEED_LOW  12500


typedef struct
{
    uint32_t LABEL   : 8 ;
    uint32_t ID      : 2 ;
    uint32_t DATA    : 19;
    uint32_t SSM     : 2 ;
    uint32_t PARITY  : 1 ;

} arinc429_bnr_t;

typedef struct 
{
    uint32_t LABEL    : 8 ;
    uint32_t ID       : 2 ;
    uint32_t DIGITAL4 : 4 ;
    uint32_t DIGITAL3 : 4 ;
    uint32_t DIGITAL2 : 4 ;
    uint32_t DIGITAL1 : 4 ;
    uint32_t DIGITAL0 : 3 ;
    uint32_t SSM      : 2 ;
    uint32_t PARITY   : 1 ;
    
}arinc429_bcd_t;

typedef struct
{
    uint32_t LABEL    : 8 ;
    uint32_t ID       : 2 ;
    uint32_t flag0    : 1 ;
    uint32_t flag1    : 1 ;
    uint32_t flag2    : 1 ;
    uint32_t flag3    : 1 ;
    uint32_t flag4    : 1 ;
    uint32_t flag5    : 1 ;
    uint32_t flag6    : 1 ;
    uint32_t flag7    : 1 ;
    
    uint32_t flag8    : 1 ;
    uint32_t flag9    : 1 ;
    uint32_t flag10   : 1 ;
    uint32_t flag11   : 1 ;
    uint32_t flag12   : 1 ;
    uint32_t flag13   : 1 ;
    uint32_t flag14   : 1 ;
    uint32_t flag15   : 1 ;
    uint32_t flag16   : 1 ;
    uint32_t flag17   : 1 ;
    uint32_t flag18   : 1 ;

    uint32_t SSM      : 2 ;
    uint32_t PARITY   : 1 ;
} arinc429_dicrete_t;


typedef union{
    arinc429_bnr_t bnr;
    arinc429_bcd_t bcd;
    arinc429_dicrete_t discrete;
    uint32_t raw;
} arinc429_word_t;

typedef struct{
    GPIO_TypeDef *port;
    uint16_t PIN_A;
    uint16_t PIN_B;
} arinc429_pins_t;

typedef struct{
    arinc429_pins_t pins;
    arinc429_word_t tx;
    arinc429_word_t rx;
    uint32_t speed;
} arinc429_t;

void arinc429_calc_parity(arinc429_t *arinc429);
void arinc429_send_packet(arinc429_t *arinc429);
void arinc429_send_bit(arinc429_t *arinc429, uint8_t bit);
void arinc429_init(arinc429_t *arinc429, GPIO_TypeDef *port, uint8_t pin_a, uint8_t pin_b);
#endif