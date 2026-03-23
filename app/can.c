#include "can.h"
#include "string.h"
#include "led.h"
//-- Variables -----------------------------------------------------------------
static uint32_t OK_MODATAL = 0;
static uint32_t ERR_MODATAL = 0;
static uint32_t OK_MODATAH = 0;
static uint32_t ERR_MODATAH = 0;
volatile static uint32_t IRQ_COUNT = 0;

void CAN_Init(void){
    RCU->HCLKCFG_bit.GPIOBEN = 1;
    RCU->HRSTCFG_bit.GPIOBEN = 1;
    GPIOB->ALTFUNCSET = 0xF000;
    GPIOB->DENSET = 0xF000;
    RCU->HCLKCFG_bit.CANEN = 1;
    RCU->HRSTCFG_bit.CANEN = 1;
    CAN->CLC_bit.DISR = 0x00;
    while((CAN->CLC_bit.DISS) & (CAN->PANCTR_bit.PANCMD)){};
    CAN -> FDR =  (0x1 << CAN_FDR_DM_Pos) | (0x3FF << CAN_FDR_STEP_Pos);   
    CAN->Node[0].NCR = CAN_Node_NCR_CCE_Msk | CAN_Node_NCR_INIT_Msk;
    CAN ->Node[0].NBTR = (0x2 << CAN_Node_NBTR_TSEG2_Pos) | (0xF << CAN_Node_NBTR_TSEG1_Pos) |
                                (0x2 << CAN_Node_NBTR_SJW_Pos) | (0x4 << CAN_Node_NBTR_BRP_Pos);                
    CAN->Node[0].NIPR = (0xC << CAN_Node_NIPR_TRINP_Pos); 
    CAN->Node[0].NCR = CAN_Node_NCR_TRIE_Msk;
  
    NVIC_EnableIRQ(CAN12_IRQn);
    
}

void CAN_Object_Location(uint32_t obj_first_num, uint32_t obj_last_num, uint32_t list_num){
    
    for(unsigned int x = obj_first_num; x <= obj_last_num; x++){
        CAN ->PANCTR = (0x2 << CAN_PANCTR_PANCMD_Pos) | 
        (x << CAN_PANCTR_PANAR1_Pos) | (list_num << CAN_PANCTR_PANAR2_Pos);
        // Статическое распределение обьектов по списку
        while(CAN->PANCTR_bit.BUSY | CAN->PANCTR_bit.RBUSY){}; // ожидание завершение команды, команды чтения/записи
    }

}


void CAN_Object_Config(uint32_t obj_num, CAN_Operation_TypeDef op_type, CAN_Message_TypeDef msg_type){

    if(op_type == CAN_OPERATION_TX){
        if(msg_type == CAN_MESSAGE_COMMON){
            CANMSG ->Msg[obj_num].MOCTR = CANMSG_Msg_MOCTR_SETDIR_Msk | 
            CANMSG_Msg_MOCTR_SETTXEN0_Msk | CANMSG_Msg_MOCTR_SETTXEN1_Msk; // как передатчик
        }
        else if( msg_type == CAN_MESSAGE_REMOTE){
            CANMSG -> Msg[obj_num].MOCTR = CANMSG_Msg_MOCTR_RESDIR_Msk |
                CANMSG_Msg_MOCTR_SETTXEN0_Msk |CANMSG_Msg_MOCTR_SETTXEN1_Msk; // отправляет запрос и ждет ответа
        }
    }
    else if(op_type == CAN_OPERATION_RX){
        if(msg_type == CAN_MESSAGE_COMMON){
            CANMSG ->Msg[obj_num].MOCTR = CANMSG_Msg_MOCTR_RESDIR_Msk | CANMSG_Msg_MOCTR_SETRXEN_Msk; // как приемник
        }
        else if(msg_type == CAN_MESSAGE_REMOTE){
            CANMSG ->Msg[obj_num].MOCTR = CANMSG_Msg_MOCTR_SETDIR_Msk | CANMSG_Msg_MOCTR_SETRXEN_Msk; // как передатчик отправляет ответ на запрос
        }
    }
    else if(op_type == CAN_OPERATION_TXRX){
        if(msg_type == CAN_MESSAGE_COMMON){
            CANMSG ->Msg[obj_num].MOCTR = CANMSG_Msg_MOCTR_SETDIR_Msk | CANMSG_Msg_MOCTR_SETTXEN0_Msk |
                                         CANMSG_Msg_MOCTR_SETTXEN1_Msk | CANMSG_Msg_MOCTR_SETRXEN_Msk;
        }
        else if(msg_type == CAN_MESSAGE_REMOTE){
            CANMSG ->Msg[obj_num].MOCTR = CANMSG_Msg_MOCTR_RESDIR_Msk | CANMSG_Msg_MOCTR_SETTXEN0_Msk |
                                         CANMSG_Msg_MOCTR_SETTXEN1_Msk | CANMSG_Msg_MOCTR_SETRXEN_Msk;
        }
    }

}

void CAN_Object_Transmit(uint32_t obj_num){
    if((CANMSG ->Msg[obj_num].MOSTAT >> CANMSG_Msg_MOSTAT_DIR_Pos) & 0x01){
        CANMSG ->Msg[obj_num].MOCTR = CANMSG_Msg_MOCTR_SETTXRQ_Msk | CANMSG_Msg_MOCTR_SETMSGVAL_Msk;
    }
}

void CAN_Object_Receive(uint32_t obj_num){
    if(!((CANMSG ->Msg[obj_num].MOSTAT >> CANMSG_Msg_MOSTAT_DIR_Pos) & 0x01)){
        CANMSG->Msg[obj_num].MOCTR = CANMSG_Msg_MOCTR_SETTXRQ_Msk | CANMSG_Msg_MOCTR_SETMSGVAL_Msk;
    }
}

void CAN_CompareData(uint32_t tx_obj_num, uint32_t rx_obj_num){
    if(CANMSG->Msg[tx_obj_num].MODATAL != CANMSG ->Msg [rx_obj_num].MODATAL){
        ERR_MODATAL ++;
    }
    else{
        OK_MODATAL ++;
    }
    if(CANMSG->Msg[tx_obj_num].MODATAH != CANMSG ->Msg [rx_obj_num].MODATAH){
        ERR_MODATAH++;
    }
    else{
        OK_MODATAH++;
    }
}

void CAN_CompareDataBlock(uint32_t first_tx_obj_num, uint32_t first_rx_obj_num, uint32_t trans_total){
    for(int x = 0; x < trans_total; x++){
        CAN_CompareData(first_tx_obj_num, first_rx_obj_num);
        first_tx_obj_num++; first_rx_obj_num++;
    }
}

void CAN_ReadData(uint32_t obj_num){
    CAN->PANCTR = (0x3 << CAN_PANCTR_PANCMD_Pos) | (obj_num << CAN_PANCTR_PANAR1_Pos);  
    while(CAN->PANCTR_bit.BUSY || CAN->PANCTR_bit.RBUSY){};
    uint32_t ID = CANMSG->Msg[obj_num].MOAR_bit.ID;
    uint32_t DLC_DATA;
    uint32_t DATA_LOW;
    uint32_t DATA_HIGH;
    uint32_t newMsg  = CANMSG->Msg[obj_num].MOSTAT_bit.NEWDAT;
    if(newMsg){
         DLC_DATA =  CANMSG->Msg[obj_num].MOFCR_bit.DLC;
         DATA_LOW = CANMSG->Msg[obj_num].MODATAL;
         DATA_HIGH = CANMSG->Msg[obj_num].MODATAH;
         printf("ID: %d DLC DATA: %d BYTE DATAHIGH: %d DATALOW: %d \n",
        (int)(ID) ,(int)(DLC_DATA), (int)(DATA_HIGH), (int)DATA_LOW);
        CANMSG->Msg[obj_num].MOCTR = CANMSG_Msg_MOCTR_RESNEWDAT_Msk;
    }
    else{
        printf("ID: %d DLC DATA: %d BYTE DATAHIGH: NO NEW DATA DATALOW: NO NEW DATA \n",
        (int)(ID) ,(int)(DLC_DATA));
    }
    
}





void CAN2_IRQHandler(void)
{
    printf("CAN INTERRUPT NODE 1\n");
    IRQ_COUNT++;
}

// nodes
void CAN12_IRQHandler(void)
{   
    if( (CAN->Node[0].NSR & (1 << CAN_Node_NSR_RXOK_Pos)) && 
    (CANMSG->Msg[OBJECT_ONE_CAN_RX].MOSTAT & CANMSG_Msg_MOSTAT_NEWDAT_Msk) )
    {
    processCommandFlag = 1;
    CANMSG->Msg[OBJECT_ONE_CAN_RX].MOCTR = CANMSG_Msg_MOCTR_RESNEWDAT_Msk;
    CAN->Node[0].NSR &= ~(1 << CAN_Node_NSR_RXOK_Pos);  
}
}


uint8_t CAN_Test(void){
    while (CAN->PANCTR_bit.BUSY) {};

    CAN_Object_Location(OBJECT_ONE_CAN_RX, OBJECT_ONE_CAN_RX, 1);     

    CAN_Object_Config(OBJECT_ONE_CAN_RX, CAN_OPERATION_RX, CAN_MESSAGE_COMMON);
    CANMSG->Msg[OBJECT_ONE_CAN_RX].MOAR = (0x1 << CANMSG_Msg_MOAR_PRI_Pos) | CANMSG_Msg_MOAR_IDE_Msk |
                                    (0x12345678 << CANMSG_Msg_MOAR_ID_Pos);
    CANMSG->Msg[OBJECT_ONE_CAN_RX].MOAMR = (0x1 << CANMSG_Msg_MOAMR_MIDE_Pos) |
                                    (0x1FFFFFFF << CANMSG_Msg_MOAMR_AM_Pos);
    CANMSG->Msg[OBJECT_ONE_CAN_RX].MOFCR = (0x8 << CANMSG_Msg_MOFCR_DLC_Pos) |
                                           CANMSG_Msg_MOFCR_TXIE_Msk |
                                           CANMSG_Msg_MOFCR_RXIE_Msk;
    CANMSG->Msg[OBJECT_ONE_CAN_RX].MOIPR = (0x2 << CANMSG_Msg_MOIPR_RXINP_Pos); 
    CAN_Object_Receive(OBJECT_ONE_CAN_RX);

    CAN_Object_Location(OBJECT_TWO_CAN_TX, OBJECT_TWO_CAN_TX, 1);
     CAN_Object_Config(OBJECT_TWO_CAN_TX, CAN_OPERATION_TX, CAN_MESSAGE_COMMON);
    CANMSG->Msg[OBJECT_TWO_CAN_TX].MOAR = (0x1 << CANMSG_Msg_MOAR_PRI_Pos) | CANMSG_Msg_MOAR_IDE_Msk;
    CANMSG->Msg[OBJECT_TWO_CAN_TX].MOAMR = ( 0x1 << CANMSG_Msg_MOAMR_MIDE_Pos);
    CANMSG->Msg[OBJECT_TWO_CAN_TX].MOIPR = (0x3 << CANMSG_Msg_MOIPR_TXINP_Pos);
    CANMSG->Msg[OBJECT_TWO_CAN_TX].MOFCR = (0x8 << CANMSG_Msg_MOFCR_DLC_Pos) |
                                           CANMSG_Msg_MOFCR_TXIE_Msk |
                                           CANMSG_Msg_MOFCR_RXIE_Msk;
    CAN_Object_Transmit(OBJECT_TWO_CAN_TX);
   
}
void CAN_Check_Errors(uint8_t nodeNum){
    uint32_t NSR_register = CAN->Node[nodeNum].NSR;
    if(NSR_register & CAN_Node_NSR_EWRN_Msk){
        printf("Node %lu => ERROR WARNING\n", nodeNum); //  lot of errors, but working (just warning)
    }
    else if (NSR_register & CAN_Node_NSR_ALERT_Msk){ 
        printf("Node %lu => ERROR ALERT\n", nodeNum);

        printf("CAN Alert! Last error code: %lu\n", 
            CAN->Node[nodeNum].NSR >> CAN_Node_NSR_LEC_Pos & 0x7); //difficult problem. reduce the load and check errors
    }
    else if(NSR_register & CAN_Node_NSR_BOFF_Msk){
        printf("Node %lu => BUS OFF: NEED TO RESET\n"); // STOP WORK OF NODE! REQUIRED MANUAL INTERVENTION (INIT MODE)
    }
    else if (NSR_register & CAN_Node_NSR_TXOK_Msk){
        printf("Node %lu => TX_OK", nodeNum);
    }
    else if(NSR_register & CAN_Node_NSR_RXOK_Msk){
        printf("Node %lu => RX-OK", nodeNum);
    }
}


void CAN_Handler(DS18B20_State *ds18b20State){
        uint8_t command = (CANMSG->Msg[OBJECT_ONE_CAN_RX].MODATAL  >> CANMSG_Msg_MODATAL_DB0_Pos) & 0xFF;
        switch ((command))
        {
        case 0x01:
            if(ds18b20State->CurrentState == STATE_IDLE){
                ds18b20State->CurrentState = STATE_MEASURING;
            }
            else{
                printf("SYSTEM => DS18B20 busy\n");
            }
            break;
        case 0x02:
            Led_Pin_ON();
            break;
        case 0x03:
            Led_Pin_OFF();
            break;
        default:
            break;
        }
}