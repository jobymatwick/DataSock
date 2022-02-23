/* 
 * File:    logger.h
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-02-15
 * Desc:    Responsible for periodically sampling data and logging it to the SD
 *            card and/or transmitting over Bluetooth. Data is sampled in a
 *            timer ISR
 */

#pragma once

#include <Arduino.h>

#define LOGGER_MAX_ADC_CHANNELS 16

typedef struct log_entry_t
{
    uint32_t time;          // Seconds since epoch
    uint16_t millis;        // Milliseconds into current second
    float    mpu_accel[3];  // MPU accelerometer X, Y, Z (m/s^2)
    float    mpu_gyro[3];   // MPU gyro X, Y, Z (rad/s)
    float    mpu_temp;      // MPU temperature (degC)

    // Array of raw 13-bit ADC readings
    uint16_t adc_data[LOGGER_MAX_ADC_CHANNELS];
} log_entry_t;

/*
 * Name:    logger_startSampling
 * Desc:    Start the timer ISR at the configured period
 */
void logger_startSampling();

/*
 * Name:    logger_serviceBuffer
 * Desc:    Attempt to write the next sample to the SD card as a CSV row
 */
void logger_serviceBuffer();
