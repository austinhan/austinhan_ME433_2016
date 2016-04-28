#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<math.h>
#include "i2c2.h"
#include "spi.h"

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

#define SLAVE_ADDR 0b0100000

void setVoltage(unsigned char channel, unsigned char voltage) {
    CS = 0;
    SPI1_IO((channel << 7 | 0b01110000)|(voltage >> 4));
    SPI1_IO(voltage << 4);
    //SPI1_IO(0b11111000);
    //SPI1_IO(0b00000001);
    int count = _CP0_GET_COUNT();
    while(_CP0_GET_COUNT() < count + 24000000/10000) {
        ;
    }
    CS = 1;
}

void initExpander(void) {
    i2c_master_start();
    i2c_master_send((SLAVE_ADDR << 1) | 0); // bit 0 becomes 0 for write
    i2c_master_send(0x00); // IODIR register address
    i2c_master_send(0b10000000);
    i2c_master_stop();
}
void setExpander(char pin, char level) {
    i2c_master_start();
    i2c_master_send((SLAVE_ADDR << 1) | 0); // bit 0 becomes 0 for write
    i2c_master_send(0x0A); // OLAT register address
    i2c_master_send(level << pin);
    i2c_master_stop();
}
char getExpander(void) {
    i2c_master_start();
    i2c_master_send((SLAVE_ADDR << 1) | 0); // bit 0 becomes 0 for write
    i2c_master_send(0x09); // GPIO register address
    i2c_master_restart();
    i2c_master_send((SLAVE_ADDR << 1) | 1); // bit 0 becomes 1 for read
    char r = i2c_master_recv();
    i2c_master_ack(1); // tell slave no more bytes requested
    i2c_master_stop(); // stop bit
    return r;
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
    initSPI1();
    initI2C2();
    initExpander();
    
    __builtin_enable_interrupts();
    int count = 0;
    while(1) {
        if((getExpander() & 0b10000000) == 0b10000000) { // if pin 7 high
            setExpander(0,1); // set pin 0 high
        } else {
            setExpander(0,0); // set pin 0 low
        }
        unsigned char va = 0;
        unsigned char vb = 0;
        int period = 2400000;
        _CP0_SET_COUNT(0);
        va = 127 + 127*sin(2*3.14159*count*500);  // typecasting?
        vb = 255*count/1000;
        setVoltage(0,va);
        setVoltage(1,vb); 
        while(_CP0_GET_COUNT() < 24000000/10000) {
            ;
        }
        count = count + 1;
        if(count >= 1000) {
            count = 0;
        }
    }
}