#ifndef SPI_H
#define SPI_H

#include <avr/io.h>
#include <stdbool.h>

#define SPI_NOT_READY !(SPSR & (1<<SPIF))

void spi_init (bool master, uint8_t cpol, uint8_t phase);
#endif