/* 
 * File:    adc.h
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-24
 * Desc:    Teensy ADC interface module.
 */

#pragma once

#include <Arduino.h>

/*
 * Name:    adc_init
 * Desc:    Configure the Teensy's ADC to operate at 13-bit resolution.
 */
void adc_init(void);

/*
 * Name:      adc_sample
 *  channels: array of channels to sample from
 *  count:    number of channels to sample
 * Desc:      Sample from multiple ADC channels. Indexes passed into `channels`
 *              array will be replaced with the corresponding channel's reading.
 */
void adc_sample(uint16_t* channels, uint8_t count);

/*
 * Name:    adc_console
 *  argc:   number of arguments
 *  argv:   list of arguments
 * Desc:    adc console command handler
 */
bool adc_console(uint8_t argc, char* argv[]);
