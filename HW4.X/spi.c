#include <xc.h>
#include "spi.h"

void initSPI1(void) {
    SDI1Rbits.SDI1R = 0b0100; // SDI1
    RPB13Rbits.RPB13R = 0b0011; // SDO1
    RPB15Rbits.RPB15R = 0b0011; // SS1
    ANSELBbits.ANSB15 = 0;
    TRISBbits.TRISB15 = 0;
    CS = 1;
    
    SPI1CON = 0;
    SPI1BUF;
    SPI1BRG = 0x7CF; // SP1BRG = 7CF 1999 for debug (20kHz); SPI1BRG =(80000000/(2*desired))-1
    SPI1STATbits.SPIROV = 0;  // clear the overflow bit
    SPI1CONbits.CKE = 1;      // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1;    // master operation
    SPI1CONbits.ON = 1;       // turn on spi 1
}

char SPI1_IO(unsigned char o) {
    SPI1BUF = o;
    while(!SPI1STATbits.SPIRBF) { // TX? RX?
        ;
    }
    return SPI1BUF;
}