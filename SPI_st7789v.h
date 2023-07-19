/*
 * SPI_st7789v.h
 *
 *  Created on: 24 dic. 2022
 *      Author: Nestor
 */
#pragma once

#include <stdint.h>
#include "cy_result.h"
#include "cyhal_gpio.h"

#if defined(__cplusplus)
extern "C"
{
#endif

typedef struct
{
    cyhal_gpio_t MOSI;
    cyhal_gpio_t MISO;
    cyhal_gpio_t SCK;
    cyhal_gpio_t SS;
    cyhal_gpio_t dc;
    cyhal_gpio_t rst;
    uint32_t frec;
} SPI_st7789v_pins_t;

cy_rslt_t SPI_st7789v_init8(const SPI_st7789v_pins_t *pins);

void SPI_st7789v_write_reset_pin(bool value);

void SPI_st7789v_write_command(uint8_t command);

void SPI_st7789v_write_data(uint8_t data);

void SPI_st7789v_write_command_stream(uint8_t *data, int num);

void SPI_st7789v_write_data_stream(uint8_t *data, int num);

uint8_t SPI_st7789v_read_data(void);

void SPI_st7789v_read_data_stream(uint8_t *data, int num);

#if defined(__cplusplus)
}
#endif
