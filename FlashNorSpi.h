/*****************************************************************************/
/**                     SPI FLASH NOR OPERATIONS V1.0                       **/
/** Created: 21/05/2026                            IDE: Mounriver Studio    **/
/** Autor: Gustavo Pereira da Silva                License: MIT             **/
/*****************************************************************************/

#ifndef FlashNorSpi_H
#define FlashNorSpi_H


/*    DEPENDENCES    */
#include "SW_SPI.h"
/* ----------------- */

#define FLASH_SIZE_MB  Flash_JEDEC[3]

uint8_t Flash_JEDEC[4] = {0,0,0,0};
uint8_t Flash_Vendor[14];
uint8_t FVL[6][14] = {"WINBOND","GIGADEVICE","MACRONIX_MXIC","EON_Cfeon","SST/Microchip","ISSI"};
volatile uint32_t NOR_TIMEOUT=0xFFFFFFFF;


void Flash_GetJedec(){
    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x9F);                        // Read JEDEC ID
    Flash_JEDEC[0] = SPI_DATAIN(8);             // VID
    Flash_JEDEC[1] = SPI_DATAIN(8);             // PID
    Flash_JEDEC[2] = (1<<((SPI_DATAIN(8))-17)); // Capacity Mbit
    SPI_CS1_HIGH;

    Flash_JEDEC[3] = Flash_JEDEC[2]/8;

    uint8_t id=0;
    if(Flash_JEDEC[0]==0xef){ for(uint8_t i=0;i<14;i++){ Flash_Vendor[i] = FVL[0][i]; } id=1;}
    if(Flash_JEDEC[0]==0xc8){ for(uint8_t i=0;i<14;i++){ Flash_Vendor[i] = FVL[1][i]; } id=1;}
    if(Flash_JEDEC[0]==0xc2){ for(uint8_t i=0;i<14;i++){ Flash_Vendor[i] = FVL[2][i]; } id=1;}
    if(Flash_JEDEC[0]==0x1c){ for(uint8_t i=0;i<14;i++){ Flash_Vendor[i] = FVL[3][i]; } id=1;}
    if(Flash_JEDEC[0]==0xbf){ for(uint8_t i=0;i<14;i++){ Flash_Vendor[i] = FVL[4][i]; } id=1;}
    if(Flash_JEDEC[0]==0x9d){ for(uint8_t i=0;i<14;i++){ Flash_Vendor[i] = FVL[5][i]; } id=1;}

    if(!id){ Flash_Vendor[0]='J'; Flash_Vendor[1]='E'; Flash_Vendor[2]='D'; Flash_Vendor[3]='E'; Flash_Vendor[4]='C'; 
             Flash_Vendor[5]='_'; Flash_Vendor[6]='0'; Flash_Vendor[7]='x';
             if((Flash_JEDEC[0]>>4)<10){ Flash_Vendor[8]= (Flash_JEDEC[0]>>4)+'0'; }else{Flash_Vendor[8]= (Flash_JEDEC[0]>>4)+'7';}
             if((Flash_JEDEC[0]&0x0F)<10){ Flash_Vendor[9]= (Flash_JEDEC[0]&0x0F)+'0'; }else{Flash_Vendor[9]= (Flash_JEDEC[0]&0x0F)+'7';}
             Flash_Vendor[10]= 0;
    }
}


uint8_t Nor_EraseAll(){

    volatile uint32_t N_TIMEOUT=0;
    uint8_t ret=0;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x06); // Write Enable
    SPI_CS1_HIGH;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x60); // Chip Erase
    SPI_CS1_HIGH;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x05); // Read Status Register 1
    while((SPI_DATAIN(8))&1<<0){ N_TIMEOUT++; if(N_TIMEOUT>NOR_TIMEOUT){ ret=1; break;} }
    SPI_CS1_HIGH;

    return ret;
}


uint8_t Nor_EraseSector(uint16_t s){

    volatile uint32_t N_TIMEOUT=0;
    uint8_t ret=0;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x06);    // Write Enable
    SPI_CS1_HIGH;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x20);    // 4Kb Sector Erase
    SPI_DATAOUT(24,4096*s); // Sector address multiple of 4096
    SPI_CS1_HIGH;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x05);    // Read Status Register 1
    while((SPI_DATAIN(8))&1<<0){ N_TIMEOUT++; if(N_TIMEOUT>NOR_TIMEOUT){ ret=1; break;} }
    SPI_CS1_HIGH;

    return ret;
}


uint8_t Nor_EraseBlock32(uint16_t b){

    volatile uint32_t N_TIMEOUT=0;
    uint8_t ret=0;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x06);     // Write Enable
    SPI_CS1_HIGH;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x52);     // 32Kb block erase
    SPI_DATAOUT(24,32768*b); // Block address multiple of 32768
    SPI_CS1_HIGH;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x05);     // Read Status Register 1
    while((SPI_DATAIN(8))&1<<0){ N_TIMEOUT++; if(N_TIMEOUT>NOR_TIMEOUT){ ret=1; break;} }
    SPI_CS1_HIGH;

    return ret;
}


uint8_t Nor_EraseBlock64(uint16_t b){

    volatile uint32_t N_TIMEOUT=0;
    uint8_t ret=0;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x06);     // Write Enable
    SPI_CS1_HIGH;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0xD8);     // 64Kb block erase
    SPI_DATAOUT(24,65536*b); // Block address multiple of 65536
    SPI_CS1_HIGH;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x05);     // Read Status Register 1
    while((SPI_DATAIN(8))&1<<0){ N_TIMEOUT++; if(N_TIMEOUT>NOR_TIMEOUT){ ret=1; break;} }
    SPI_CS1_HIGH;

    return ret;
}


uint8_t Nor_PageProgram(uint32_t pg, uint16_t boff, uint8_t data[]){

    volatile uint32_t N_TIMEOUT=0;
    uint8_t ret=0;

    uint16_t boffset=256*boff;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x06);    // Write Enable
    SPI_CS1_HIGH;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x02);    // Page Program
    SPI_DATAOUT(24,pg*256); // Page start address pg*256
    for(uint16_t i=0;i<256;i++){ SPI_DATAOUT(8,data[i+boffset]); }
    SPI_CS1_HIGH;

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x05);    // Read Status Register 1
    while((SPI_DATAIN(8))&1<<0){ N_TIMEOUT++; if(N_TIMEOUT>NOR_TIMEOUT){ ret=1; break;} }
    SPI_CS1_HIGH;

    return ret;
}


void Nor_PageRead(uint32_t pg, uint8_t data[]){

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x03);    // Page read
    SPI_DATAOUT(24,pg*256); // Page start read address
    for(uint16_t i=0;i<256;i++){ data[i] = SPI_DATAIN(8); }
    SPI_CS1_HIGH;
}


/* SETORES DE 512 bytes - 512 bytes SECTOR */

void Nor_SectorRead(uint32_t sec, uint8_t data[]){

    SPI_CS1_LOW;
    SPI_DATAOUT(8,0x03);     // Page read
    SPI_DATAOUT(24,sec*512); // Page start read address
    for(uint16_t i=0;i<256;i++){ data[i] = SPI_DATAIN(8); }
    SPI_CS1_HIGH;
}

void Nor_SectorProgram(uint32_t sec, uint8_t data[]){

    Nor_PageProgram((2*sec), 0, data);
    Nor_PageProgram((2*sec)+1, 256, data);    
}


#endif

/****************************************************************/
/**         PARA MAIS INFORMACOES - MORE INFORMATIONS          **/
/**           https://www.youtube.com/@Gustavo_PORTD           **/
/**              https://x.com/gustavo_portd                   **/
/****************************************************************/
