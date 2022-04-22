/* 
 * File:    adc.cpp
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-24
 * Desc:    Teensy ADC interface module.
 */

#include "adc.h"

#include <stdio.h>
#include <stdlib.h>

#define ADC_RES_BITS 13

const uint8_t chan_order[] = { 9, 8, 7, 6, 3, 2, 1, 0, 19, 18, 17, 16, 15, 14, 20, 21};

// For mapping analog input labels (i.e A0, A1) to pin numbers for analogRead()
const uint8_t _analog_to_pin[] = { 
    PIN_A0,  PIN_A1,  PIN_A2,  PIN_A3,  PIN_A4,  PIN_A5,  PIN_A6,
    PIN_A7,  PIN_A8,  PIN_A9,  PIN_A10, PIN_A11, PIN_A12, PIN_A13,
    PIN_A14, PIN_A15, PIN_A16, PIN_A17, PIN_A18, PIN_A19, PIN_A20,
    PIN_A21, PIN_A22, PIN_A23, PIN_A24, PIN_A25, PIN_A26
};

uint8_t _channel_count = 1;
uint16_t _print_samples = 0;

void adc_init(void)
{
    analogReadResolution(ADC_RES_BITS);
}

void adc_sample(uint16_t* channels, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++)
        channels[i] = analogRead(_analog_to_pin[chan_order[i]]);

    if (_print_samples)
    {
        Serial.printf("[%" PRIu16, channels[0]);
        for (uint8_t i = 1; i < count; i++)
            Serial.printf(", %" PRIu16, channels[i]);
        Serial.println("]");

        _print_samples -= _print_samples > 0 ? 1 : 0;
    }
}

bool adc_console(uint8_t argc, char* argv[])
{
    if (!strcmp("init", argv[1]))
    {
        adc_init();
        Serial.println("ADC initialized!");
        return true;
    }

    if (!strcmp("sample", argv[1]))
    {
        if (argc < 3)
        {
            Serial.println("at least one channel must be sampled!");
            return false;
        }

        uint16_t channels[16];

        for (uint8_t i = 0; i < argc - 2; i++)
            channels[i] = (uint16_t) atoi(argv[i + 2]);
        
        adc_sample(channels, argc - 2);

        Serial.printf("[%" PRIu16, channels[0]);
        for (uint8_t i = 1; i < argc - 2; i++)
            Serial.printf(", %" PRIu16, channels[i]);
        Serial.println("]");

        return true;
    }

    if (!strcmp("print", argv[1]))
    {
        if (argc < 3)
        {
            Serial.println("arg count");
            return false;
        }

        _print_samples = atoi(argv[2]);
        
        return true;
    }

    return false;
}
