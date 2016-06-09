/*******************************************************************************
  MPLAB Harmony Project Main Source File

  Company:
    Microchip Technology Inc.
  
  File Name:
    main.c

  Summary:
    This file contains the "main" function for an MPLAB Harmony project.

  Description:
    This file contains the "main" function for an MPLAB Harmony project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state 
    machines of all MPLAB Harmony modules in the system and it calls the 
    "SYS_Tasks" function from within a system-wide "super" loop to maintain 
    their correct operation. These two functions are implemented in 
    configuration-specific files (usually "system_init.c" and "system_tasks.c")
    in a configuration-specific folder under the "src/system_config" folder 
    within this project's top-level folder.  An MPLAB Harmony project may have
    more than one configuration, each contained within it's own folder under
    the "system_config" folder.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

//Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "system/common/sys_module.h"   // SYS function prototypes
#include <xc.h>
#include "i2c2.h"

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
#define SLAVE_ADDR 0b1101011

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

int main ( void )
{
    /* Initialize all MPLAB Harmony modules, including application(s). */
    SYS_Initialize ( NULL );
    i2c2_init();
    initIMU();
    initPWM();
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        if(checkIMU() == 0b01101001) {
            LATAbits.LATA4 = 1;
        } else {
            LATAbits.LATA4 = 0;
        }
        SYS_Tasks ( );
        LATAbits.LATA4 = 1;
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

