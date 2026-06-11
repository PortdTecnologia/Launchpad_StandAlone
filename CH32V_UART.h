/*****************************************************************************/
/**                          CH32V SERIAL V1.0                              **/
/** Created: 04/12/2025                            IDE: Mounriver Studio    **/
/** Autor: Gustavo Pereira da Silva                PORTD Tecnologia         **/
/*****************************************************************************/

#define TXE  (USART1->STATR & 1<<7)
#define RXNE (USART1->STATR & 1<<5)
#define IDLE (USART1->STATR & 1<<4)

void UART1_Config(uint32_t baudrate){
    TRISDbits(5,Out30AF_PP);
	TRISDbits(6,Out30AF_PP);
    RCC->PB2PCENR |=(1<<14); // RCC EN
    USART1->BRR = (SystemCoreClock/baudrate);
    USART1->CTLR1 |= (1<<13); // UE
    USART1->CTLR1 |=(1<<2); USART1->CTLR1 |=(1<<3); // TX RX ENABLE
}

void UART1_Print(char str[]){

    for(uint16_t i=0; str[i]!=0;i++){
        while(!TXE);
        USART1->DATAR = str[i];
    }
    while(!TXE);
}


uint8_t uartrx[1026];
uint16_t uartrxsize=0;

void UART1_Read(){
    uartrxsize=0;
    for(uint16_t i=0;i<1026;i++){
        uartrx[i] = USART1->DATAR;
        while(!RXNE){ if(IDLE){uartrxsize=i+1; break;} }
        if(IDLE){(void)USART1->DATAR; break;}
    }
    uartrx[uartrxsize]=0;
}