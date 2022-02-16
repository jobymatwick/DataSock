/* 
 * File:    main.cpp
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-20
 * Desc:    Main program entry point.
 */

#include <Arduino.h>

#include "adc.h"
#include "console.h"
#include "mpu.h"
#include "clock.h"
#include "storage.h"

#define LED_PERIOD 100
#define CONSOLE_PERIOD 50

uint32_t next_led = LED_PERIOD;
uint32_t next_console = CONSOLE_PERIOD;
uint32_t next_adc = 250;

void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    // Init SD and load config first!
    storage_init();
    
    console_init();
    adc_init();
    mpu_init();
    clock_init();
}

void loop()
{
    if (millis() >= next_led)
    {
        next_led += LED_PERIOD;
        digitalToggle(LED_BUILTIN);
    }

    if (millis() >= next_console)
    {
        next_console += CONSOLE_PERIOD;
        console_tick(NULL);
    }

    if (millis() >= next_adc)
    {
        next_adc += storage_configGetNum(CONFIG_POLL_RATE);
        adc_readTask(NULL);
    }
}
