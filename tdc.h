#ifndef TDC_H
#define TDC_H

#include "stdint.h"

void tdc_send_command(uint8_t auto_increment, uint8_t write_flag, uint8_t address);
void tdc_write_to_spi_FIFO(uint8_t data);
uint8_t tdc_read_from_spi_FIFO();
void tdc_set_register(uint8_t address, uint8_t data);
uint8_t tdc_get_register_8b(uint8_t address);
uint32_t tdc_get_register_24b(uint8_t address);
void tdc_spi_start();
void tdc_spi_stop();
void tdc_init();
uint32_t tdc_measure();


#endif