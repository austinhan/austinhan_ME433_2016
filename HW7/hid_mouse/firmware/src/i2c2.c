#include <xc.h>
#include "i2c2.h"

// I2C Master utilities, 100 kHz, using polling rather than interrupts
// The functions must be callled in the correct order as per the I2C protocol
// Change I2C1 to the I2C channel you are using
// I2C pins need pull-up resistors, 2k-10k

void i2c2_init(void) {
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    I2C2BRG = 233;            // I2CBRG = [1/(2*Fsck) - PGD]*Pblck - 2 
                                    // look up PGD for your PIC32 = 104 ns, Fsck = 100 kHz, PBCLK = 48 MHz
    I2C2CONbits.ON = 1;               // turn on the I2C2 module
}

// Start a transmission on the I2C bus
void i2c_master_start(void) {
    I2C2CONbits.SEN = 1;            // send the start bit
    while(I2C2CONbits.SEN) { ; }    // wait for the start bit to be sent
}

void i2c_master_restart(void) {     
    I2C2CONbits.RSEN = 1;           // send a restart 
    while(I2C2CONbits.RSEN) { ; }   // wait for the restart to clear
}

void i2c_master_send(unsigned char byte) { // send a byte to slave
  I2C2TRN = byte;                   // if an address, bit 0 = 0 for write, 1 for read
  while(I2C2STATbits.TRSTAT) { ; }  // wait for the transmission to finish
  if(I2C2STATbits.ACKSTAT) {        // if this is high, slave has not acknowledged
    // ("I2C2 Master: failed to receive ACK\r\n");
  }
}

unsigned char i2c_master_recv(void) { // receive a byte from the slave
    I2C2CONbits.RCEN = 1;             // start receiving data
    while(!I2C2STATbits.RBF) { ; }    // wait to receive the data
    return I2C2RCV;                   // read and return the data
}

void i2c_master_ack(int val) {        // sends ACK = 0 (slave should send another byte)
                                      // or NACK = 1 (no more bytes requested from slave)
    I2C2CONbits.ACKDT = val;          // store ACK/NACK in ACKDT
    I2C2CONbits.ACKEN = 1;            // send ACKDT
    while(I2C2CONbits.ACKEN) { ; }    // wait for ACK/NACK to be sent
}

void i2c_master_stop(void) {          // send a STOP:
  I2C2CONbits.PEN = 1;                // comm is complete and master relinquishes bus
  while(I2C2CONbits.PEN) { ; }        // wait for STOP to complete
}

void i2c_read_multiple(char address, char reg, unsigned char *data, char length) {
    i2c_master_start();
    i2c_master_send((address << 1) | 0); // bit 0 becomes 0 for write
    i2c_master_send(reg);
    i2c_master_restart();
    i2c_master_send((address << 1) | 1); // bit 0 becomes 1 for read
    int i;
    for(i=0;i<length;i++) {
        data[i] = i2c_master_recv();
        if(i == length-1) {
            i2c_master_ack(1);
        } else {
            i2c_master_ack(0);
        }
    }
    i2c_master_stop();
}