#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include "i2c2.h"

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect     
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // free up secondary osc pins
#pragma config FPBDIV = DIV_1 // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1048576 // slowest wdt
#pragma config WINDIS = OFF // no wdt window
#pragma config FWDTEN = OFF // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz (from 8MHz crystal)
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 1 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

#define SLAVE_ADDR 0b1101011

char checkIMU(void) {
    i2c_master_start();
    i2c_master_send((SLAVE_ADDR << 1) | 0); // bit 0 becomes 0 for write
    i2c_master_send(0x0F); // GPIO register address
    i2c_master_restart();
    i2c_master_send((SLAVE_ADDR << 1) | 1); // bit 0 becomes 1 for read
    char r = i2c_master_recv();
    i2c_master_ack(1); // tell slave no more bytes requested
    i2c_master_stop(); // stop bit
    return r;
}

void initIMU(void) {
    i2c_master_start();
    i2c_master_send((SLAVE_ADDR << 1) | 0); // bit 0 becomes 0 for write
    i2c_master_send(0x10); // CTRL1_XL register address
    i2c_master_send(0b10000000);
    i2c_master_restart();
    i2c_master_send(0x11); // CTRL2_G register address
    i2c_master_send(0b10000000);
    i2c_master_restart();
    i2c_master_send(0x12); //CTRL3_C register address
    i2c_master_send(0b00000100);
    i2c_master_stop();
}

void initPWM(void) {
    
    RPB7Rbits.RPB7R = 0b0101; // OC1
    RPB8Rbits.RPB8R = 0b0101; // OC2
    
    T2CONbits.TCKPS = 0b011; // Timer3 Prescaler = 8
	PR2 = 5999;              // set to 5 kHz = 48 MHz/8/5000 - 1
	TMR2 = 0;
	OC1CONbits.OCTSEL = 0;
	OC1CONbits.OCM = 0b110;
	OC1R = 2999;
    OC1RS = 2999;
    OC2CONbits.OCTSEL = 0;
	OC2CONbits.OCM = 0b110;
	OC2R = 2999;
    OC2RS = 2999;
	T2CONbits.ON = 1;
	OC1CONbits.ON = 1;
    OC2CONbits.ON = 1;
    TRISBbits.TRISB7 = 0;
    TRISBbits.TRISB8 = 0;
    
    LATBbits.LATB4 = 1;
}

int main() {

    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;
    
    // initialize SPI1, I2C2, I/O expander 
    initI2C2();
    initIMU();
    initPWM();
    TRISAbits.TRISA4 = 0;
    TRISBbits.TRISB4 = 0;
    
    __builtin_enable_interrupts();
    
    while(1) {
        _CP0_SET_COUNT(0);
        LATAINV = 0b10000;
//        if(checkIMU() == 0b01101001) {
//            LATAbits.LATA4 = 0;
//            LATBbits.LATB4 = 1;
//        } else {
//            LATAbits.LATA4 = 0;
//            LATBbits.LATB4 = 0;
//        }
        unsigned char data[14];
        i2c_read_multiple(SLAVE_ADDR,0x20,data,14);
        short temp = (data[1] << 8) | data[0];
        short g_x  = (data[3] << 8) | data[2];
        short g_y  = (data[5] << 8) | data[4];
        short g_z  = (data[7] << 8) | data[6];
        short xl_x = (data[9] << 8) | data[8];
        short xl_y = (data[11] << 8) | data[10];
        short xl_z = (data[13] << 8) | data[12];
        
        OC1RS = xl_x*3000/16384 + 2999;
        OC2RS = xl_y*3000/16384 + 2999;
        while(_CP0_GET_COUNT() < 480000) {
            ;
        }
    }
}