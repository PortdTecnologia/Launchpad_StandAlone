#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include "32V00x_IODriver.h"
#include "CH32V00x_Timers.h"
#include "CH32V_UART.h"
#include "WavPlayer.h"


uint32_t CK(){

    volatile uint32_t v=0;
    TRISCbits(6,Out30PP);
    Delay_Ms(1);
    TRISCbits(6,InPU);
    while(PC6){v++;}
    Delay_Ms(1);
    return v;

}


int main(){

    SystemConfig();
    UART1_Config(921600); //921600
    USART1->STATR &= ~(1<<5);

    /* IOs */
    TRISCbits(0,Out30AF_PP); // AUDIO OUT

    TRISCbits(1,Out30PP); // CS
    TRISCbits(2,InPD);    // MISO
    TRISCbits(3,Out30PP); // CLK
    TRISCbits(4,Out30PP); // MOSI
    PORTC_OUT(1,1);

    TRISAbits(2,InPU); // KEYBOARD


    /* TIMER 2 */
    TIMER2_ON(0);
    TIM2->ATRLR = 255;              // ARR- FREQ
    TIM2->CH3CVR = 0;               // DUTY
    TIM2->CHCTLR2 = 0b01100000;     // MODE PWM 1
    TIM2->CHCTLR2 |= (1<<3);        // CCR PRELOAD ENABLE
    TIM2->CCER |= (1<<8);           // OUTPUT ENABLE

    Delay_Ms(500);

    //uint8_t sp[256];


    Flash_GetJedec();
    printf("Flash -> %s  %dMbit %dMegabytes\r\n",Flash_Vendor, Flash_JEDEC[2], FLASH_SIZE_MB);

    TAR_Analizer();
    Tar_PrintFiles();



    while(1){


        /* PLAYER */

        if(!PC6){

            uint32_t Key=0;
            Key = (CK());
            while(!PC6);
            Delay_Ms(1);
            printf("Key -> %d\r\n",Key);

            if(Key>600 && Key<800){ PlaySample(1); }
            //if(Key>1250 && Key<1500){ PlaySample(2); }
            //if(Key>750 && Key<950){ PlaySample(3); }

            //if(Key>2000 && Key<2700){ PlaySample(4); }
            //if(Key>6000 && Key<7500){ PlaySample(5); }

        }





        /* END PLAYER */

        /* LOADER SERIAL */

        if(RXNE){

            UART1_Read();

            uint16_t page=0;

            SPI_CS1_HIGH; Delay_Ms(1); // Interrompe SPI

            if(uartrx[0]=='D'){
                Nor_EraseAll();

                while(!TXE){} USART1->DATAR = 'O';
                while(!RXNE);
                UART1_Read();

                while(uartrx[258]==0x60){
                    Nor_PageProgram(page, 0, uartrx);
                    page++;

                    while(!TXE){} USART1->DATAR = 'O';
                    while(!RXNE);
                    UART1_Read();
                }

                Nor_PageProgram(page, 0, uartrx);
                while(!TXE){} USART1->DATAR = 'O';
            }

            Delay_Ms(1);
            NVIC_SystemReset();
        }
        /* END LOADER */


    }

}