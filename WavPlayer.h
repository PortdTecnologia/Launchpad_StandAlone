/*****************************************************************************/
/**                    Simple Wav Player for CH32V00x V1.0                  **/
/** Created: 28/05/2026                            IDE: Mounriver Studio    **/
/** Autor: Gustavo Pereira da Silva                License: MIT             **/
/*****************************************************************************/

#ifndef WavPlayer_H
#define WavPlayer_H

#include "TarAnalizer.h"


void FadeIn(){
    for(uint8_t i=0;i<127;i++){
        TIM2->CH3CVR = i; // DUTY
        Delay_Us(100);
    }
}
void FadeOut(){
    for(uint8_t i=127;i;i--){
        TIM2->CH3CVR = i; // DUTY
        Delay_Us(100);
    }
}


void PlaySample(uint8_t f){

    if(f){

        Tar_fopen(f);
        
        SPI_CS1_LOW;
        SPI_DATAOUT(8,0x03);
        SPI_DATAOUT(24,(T_File*512) +44);
        
        FadeIn();
        for(uint32_t i=0;i<T_FILES[f-1][1]-100;i++){
            TIM2->CH3CVR = SPI_DATAIN(8); // DUTY
            Delay_Us(15);
            if(!PC6){break;}
        }
        FadeOut();

        SPI_CS1_HIGH;
    }
}



/****************************************************************/
/**         PARA MAIS INFORMACOES - MORE INFORMATIONS          **/
/**           https://www.youtube.com/@Gustavo_PORTD           **/
/**              https://x.com/gustavo_portd                   **/
/****************************************************************/

#endif