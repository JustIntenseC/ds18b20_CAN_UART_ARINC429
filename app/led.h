#include "K1921VK035.h"
#include "stdint.h"
typedef enum{
    STATUS_OFF,
    STATUS_ON
} LED_STATUS;

LED_STATUS Led_Pin_ON(void);
LED_STATUS Led_Pin_OFF(void);
LED_STATUS Led_Pin_ON_CAN(void);
LED_STATUS Led_Pin_ON_CAN(void);
void Led_Pin_Toggle(void);
void Led_Pin_Init(void);