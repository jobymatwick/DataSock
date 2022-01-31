/* 
 * File:    main.cpp
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-20
 * Desc:    Main program entry point.
 */

#include <Arduino.h>
#include <arduino-timer.h>

#include "adc.h"
#include "console.h"
#include "mpu.h"
#include "clock.h"

#define TIMER_TASKS 5
#define LED_PIN 13

Timer<TIMER_TASKS> timer;

/*
 * Name:    toggleLed
 * Desc:    toggle the LED on LED_PIN
 */
bool toggleLed(void *unused);

void setup()
{
    pinMode(LED_PIN, OUTPUT);

    adc_init();
    console_init();
    mpu_init();
    clock_init();

    // Setup recurring tasks
    timer.every(100, toggleLed);
    timer.every(50, console_tick);
    timer.every(250, adc_readTask);
}

void loop()
{
    // Repeatedly tick the timer
    timer.tick();
}

bool toggleLed(void *unused)
{
    static uint8_t last_state = 0;
    digitalWrite(LED_PIN, !last_state);
    last_state = !last_state;
    
    return true;
}
