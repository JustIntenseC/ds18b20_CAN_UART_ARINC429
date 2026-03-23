
#include "main.h"
#include "uart.h"
#include "ds18b20.h"
#include "led.h"
#include "button.h"
#include "can.h"
#include "retarget_conf.h"
volatile uint8_t timerDone = 1;
volatile uint8_t gpioFlag = 0;
uint8_t flagTMR1 = 0;


    int main(void){
    SystemCoreClockUpdate(); // 100 [МГц]
    SysTick_Config(100000);
    Led_Pin_Init();
    Button_Init();
    GPIO_Init();
    Timer0_Init();
    Timer1_Init();
    USB_UART_Init();
    DS18B20_Reset();
    DS18B20_Mode ds18b20Mode = {
        .CurrentMode =  MODE_IDLE
    };
    DS18B20_State ds18b20State ={
        .CurrentState = STATE_IDLE,
        .scratchpad = {0},
        .rom_address = {0},
        .type_msg = CAN_MSG,
        .object_RX = OBJECT_ONE_CAN_RX,
        .object_TX = OBJECT_TWO_CAN_TX
    };

    printf("\nAll peripherals inited, SYSCLK = %3d MHz\n", (int)(SystemCoreClock / 1E6)); 
    printf("Waiting, press BUTTON to select mode of work\n");

    CAN_Init();
    CAN_Test();


    HandleModeDS18B20(&ds18b20Mode, &ds18b20State);
    while(1){
        if(gpioFlag){
            if(ds18b20State.CurrentState != STATE_ERROR){
                ds18b20State.CurrentState = STATE_IDLE;
            }
            selectMode(&ds18b20Mode.CurrentMode);
            HandleModeDS18B20(&ds18b20Mode, &ds18b20State);
            gpioFlag = 0;
        }
        while(!gpioFlag){
            HandleModeDS18B20(&ds18b20Mode, &ds18b20State);
            if(processCommandFlag){
                ProcessCommand((char*)packet_buffer, &ds18b20State, &ds18b20Mode);
                memset(packet_buffer, 0, sizeof(packet_buffer));
                processCommandFlag = 0;
            }
        }
        
    }
    return 0;
}


void SysTick_Handler(void){
    msticks++;
}

void GPIOA_IRQHandler(void){
    GPIOA->INTSTATUS_bit.PIN7 = 1;
    static uint32_t last_time = 0;
    uint32_t time_now = msticks;
    if(time_now - last_time > 100){
        gpioFlag = 1;
    }
    last_time = time_now;
}

static void Timer0_Init(void){
    RCU->PCLKCFG_bit.TMR0EN = 1;
    RCU->PRSTCFG_bit.TMR0EN = 1;
    TMR0->CTRL_bit.INTEN= 1;
    NVIC_SetPriority(TMR0_IRQn, 0);
    NVIC_EnableIRQ(TMR0_IRQn);
    TMR0->CTRL_bit.ON = 0 ;
}

void TMR0_IRQHandler(void){
    TMR0->INTSTATUS = 1;
    TMR0->LOAD = 0;
    timerDone = 1;
    TMR0 -> CTRL_bit.ON = 0;
}

void Timer_Delay_Ticks(uint32_t ticks){
    TMR0->VALUE = 0;
    TMR0 ->LOAD = ticks * 100;
    TMR0->CTRL_bit.ON = 1;
    while (!timerDone){};
    timerDone = 0;
}

static void Timer1_Init(void){
    RCU->PCLKCFG_bit.TMR1EN = 1;
    RCU->PRSTCFG_bit.TMR1EN = 1;
    TMR1->CTRL_bit.INTEN = 1;
    NVIC_SetPriority(TMR1_IRQn, 0);
    NVIC_EnableIRQ(TMR1_IRQn);
    TMR1->CTRL_bit.ON = 0 ;
}
void Timer_Delay_Ticks_NoneBLocking(uint32_t ticks){
    if(TMR1->CTRL_bit.ON == 0){
        TMR1 -> VALUE = 0;
        TMR1 -> LOAD = ticks;
        TMR1 -> CTRL_bit.ON = 1;
    }
}
void TMR1_IRQHandler(void){
    flagTMR1 = 0;
    TMR1->INTSTATUS = 1;
    TMR1 -> LOAD =  0;
    TMR1 -> CTRL_bit.ON = 0;
}



void ProcessCommand(char *msg, DS18B20_State* ds18b20State, DS18B20_Mode* ds18b20Mode){
    if(ds18b20Mode->CurrentMode == MODE_MANUAL){


        switch (ds18b20State->type_msg){
            case UART_MSG:
            if(strncmp(msg, "led on\n", 7) == 0){
                Led_Pin_ON();
            }      
            else if (strncmp(msg, "led off\n", 8) == 0){
                Led_Pin_OFF();
            }
            else if(strncmp(msg, "temp\n", 5)==0){
                if(ds18b20State->CurrentState == STATE_IDLE){
                    ds18b20State->CurrentState = STATE_MEASURING;
                }
                else{ printf("SYSTEM => DS18B20 busy\n"); }
            }
            else if(strncmp(msg, "typemsg: can\n", strlen("typemsg: can\n"))==0){
                ds18b20State->CurrentState == STATE_IDLE;
                ds18b20State->type_msg = CAN_MSG;
            }
                break;
            case CAN_MSG:
                if(strncmp(msg, "typemsg: uart\n", strlen("typemsg: uart\n"))==0){
                    ds18b20State->CurrentState == STATE_IDLE;
                    ds18b20State->type_msg = UART_MSG;
                    break;
                }
                CAN_Handler(ds18b20State);
                break;
            case ARINC429_MSG:
                break;
            default:
                break;
        }
    }
    if(ds18b20Mode->CurrentMode == MODE_AUTO){
        if(strncmp(msg, "temp\n", 5) == 0){
            ds18b20State->CurrentState = STATE_MEASURING;
        }
    }
    if(strncmp(msg, "reset\n", 6) == 0){
        ds18b20Mode->CurrentMode = MODE_IDLE;
        ds18b20State->CurrentState = STATE_IDLE;
        if((GPIOA->DATA >> 8) & 0x01 == 1) Led_Pin_OFF();
        printf("SYSTEM => RESET\n");
        printf("Waiting, press BUTTON to select mode of work\n");
    }
}




static void GPIO_Init(void){
    RCU->HCLKCFG_bit.GPIOBEN = 1;
    RCU->HRSTCFG_bit.GPIOBEN = 1;
    GPIOB->ALTFUNCSET_bit.PIN8 = 1; // TRANSMIT
    GPIOB->OUTMODE_bit.PIN8 = GPIO_OUTMODE_PIN8_PP;
    GPIOB->DENSET_bit.PIN8 = 1;
    GPIOB->ALTFUNCSET_bit.PIN9 = 1; // RECEIVE
    GPIOB->OUTMODE_bit.PIN9 = GPIO_OUTMODE_PIN9_PP;
    GPIOB->DENSET_bit.PIN9 = 1;

    //DS18B20 SIGNAL
    GPIOB->DENSET_bit.PIN0 = 1;
    GPIOB->OUTMODE_bit.PIN0 = GPIO_OUTMODE_PIN0_OD;
    GPIOB->OUTENSET_bit.PIN0 = 1;
    GPIOB->DATAOUTSET_bit.PIN0 = 1; // OFF OPEN DRAIN

    //3.3v 
    GPIOB -> DENSET_bit.PIN1 = 1;
    GPIOB-> OUTMODE_bit.PIN1 = GPIO_OUTMODE_PIN1_PP;
    GPIOB-> OUTENSET_bit.PIN1 = 1;
    GPIOB->DATAOUTSET_bit.PIN1 = 1;

    // GND
    GPIOB -> DENSET_bit.PIN2 = 1;      
    GPIOB -> OUTMODE_bit.PIN2 = GPIO_OUTMODE_PIN2_OD;  
    GPIOB -> OUTENSET_bit.PIN2 = 1;   

    //ARINC 429
    // GPIOB ->DENSET |= (1 << GPIO_DENSET_PIN3_Pos) | (1 << GPIO_DENSET_PIN4_Pos);
    // GPIOB->OUTMODE |= (GPIO_OUTMODE_PIN3_PP << GPIO_OUTMODE_PIN3_Pos) | 
    //                 (GPIO_OUTMODE_PIN4_PP << GPIO_OUTMODE_PIN4_Pos) ;
    // GPIOB->OUTENSET |= (1 << GPIO_OUTENSET_PIN3_Pos) | (1 << GPIO_OUTENSET_PIN4_Pos);
}

void HandleEventDS18B20(DS18B20_State *ds18b20State){
    switch (ds18b20State->CurrentState)
    {
    case STATE_IDLE:
        // waiting
        break;
    case STATE_MEASURING:
        if(DS18B20_Reset() == STATE_ERROR){
            ds18b20State->CurrentState = STATE_ERROR;
            break;
        };
        DS18B20_WriteByte(0xCC);
        DS18B20_WriteByte(0x44);
        start = msticks;
        ds18b20State->CurrentState = STATE_WAIT_MEASURING;
        break;

    case STATE_WAIT_MEASURING:
        if(msticks - start >= DS18B20_MEASURE_DELAY){
            ds18b20State->CurrentState = STATE_READING;
        }
        break;

    case STATE_READING:
        if(DS18B20_Reset() == STATE_ERROR){
            ds18b20State->CurrentState = STATE_ERROR;
            break;
        };
        DS18B20_WriteByte(0xCC);
        DS18B20_WriteByte(0xBE);
        for (uint8_t i = 0; i < 9; i++) {
            ds18b20State->scratchpad[i] = DS18B20_ReadByte();
        }
        if(DS18B20_CRC8(ds18b20State->scratchpad, 8) != ds18b20State->scratchpad[8]){
            ds18b20State->CurrentState = STATE_ERROR;
            break;
        }; 
        ds18b20State->CurrentState = STATE_READ_DONE;
        msticks = 0;
        start = 0;
        break;

    case STATE_ERROR:
        break;
    }
}

void HandleModeDS18B20(DS18B20_Mode* ds18b20Mode, DS18B20_State *ds18b20State){
    switch (ds18b20Mode->CurrentMode){

    case MODE_IDLE:
    //
        break;
    

    case MODE_MANUAL:
        HandleEventDS18B20(ds18b20State);
        if(ds18b20State->CurrentState == STATE_READ_DONE){   
            switch (ds18b20State->type_msg)
            {
            case UART_MSG:
                DS18B20_SendMessage(ds18b20State);
                break;
            
            case CAN_MSG:
                DS18B20_SendMessage_CAN(ds18b20State);


            case ARINC429_MSG:
                //
                break;

            default:
                break;
            }
            
        }
        if(ds18b20State->CurrentState == STATE_ERROR) ds18b20Mode->CurrentMode = MODE_ERROR;
        break;

    case MODE_AUTO:
        Timer_Delay_Ticks_NoneBLocking(200000000);
        if(!flagTMR1){
            if(ds18b20State->CurrentState == STATE_IDLE){
                ds18b20State->CurrentState = STATE_MEASURING;
                flagTMR1 = 1;
            }
        }
        if(ds18b20State ->CurrentState != STATE_ERROR) HandleEventDS18B20(ds18b20State);
        
        if(ds18b20State->CurrentState == STATE_READ_DONE){
            switch (ds18b20State->type_msg)
            {
            case UART_MSG:
                DS18B20_SendMessage(ds18b20State);
                break;
            
            case CAN_MSG:
                DS18B20_SendMessage_CAN(ds18b20State);


            case ARINC429_MSG:
                //
                break;

            default:
                break;
            }
        }

        if(ds18b20State->CurrentState == STATE_ERROR) ds18b20Mode->CurrentMode = MODE_ERROR;
        break;

    case MODE_ERROR:
        Error_Handler();
        break;
    default:
        break;
    }
}

void selectMode(STATE_MODE *currentMode){
    switch (*currentMode)
    {
    case MODE_IDLE:
        printf("Selected manual mode of work\n");
        *currentMode =  MODE_MANUAL;
        break;
    case MODE_MANUAL:
        printf("Selected auto mode of work\n");
        *currentMode = MODE_AUTO;
        break;
    case MODE_AUTO:
        printf("Selected manual mode of work\n");
        *currentMode = MODE_MANUAL;
        break;
    
    default:
        break;
    }
}




void Error_Handler(void){
        Led_Pin_Toggle();
        Timer_Delay_Ticks(250000);
    
}


















