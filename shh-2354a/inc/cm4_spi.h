// inc/cm4_spi.h

#ifndef CM4_SPI_H
#define CM4_SPI_H

void init_cm4_spi(void);
void cm4_spi_enable(void);
void cm4_spi_disable(void);
volatile uint8_t *cm4_spi_get_ram(void);

#endif