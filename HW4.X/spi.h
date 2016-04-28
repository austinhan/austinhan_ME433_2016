#ifndef SPI_H    /* Guard against multiple inclusion */
#define SPI_H
#define CS LATBbits.LATB15

void initSPI1(void);
char SPI1_IO(unsigned char);

#endif /* _EXAMPLE_FILE_NAME_H */

/* *****************************************************************************
 End of File
 */
