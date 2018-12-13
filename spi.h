/* (c) 2018 ukrkyi */
#ifndef SPI_H
#define SPI_H

#include <stdbool.h>
#include <stdint.h>

extern void spi_init(int count, int div);
extern bool spi_send(uint16_t data);
extern bool spi_can_send();
extern bool spi_is_tx();
extern void spi_tx_cplt();

#endif // SPI_H
