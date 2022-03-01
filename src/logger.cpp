/* 
 * File:    logger.cpp
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-02-15
 * Desc:    Responsible for periodically sampling data and logging it to the SD
 *            card and/or transmitting over Bluetooth.
 */

#include "logger.h"

#include "adc.h"
#include "clock.h"
#include "mpu.h"
#include "storage.h"

#define CIRC_BUF_LEN 40
#define CSV_ROW_BUF_LEN 200

/*
 * Name:    _sampleISR
 * Desc:    Sample from ADC channels and MPU and store readings to buffer
 * !!! TO BE CALLED BY TIMER ISR !!!
 */
void _sampleISR();

IntervalTimer _sample_timer;

log_entry_t _circ_buf[CIRC_BUF_LEN];
log_entry_t* _head = _circ_buf;
log_entry_t* _tail = _circ_buf;
bool _running = false;

void logger_startSampling()
{
    static uint16_t last_period = 0;
    uint16_t this_period = (uint16_t) storage_configGetNum(CONFIG_POLL_RATE);

    if (this_period != last_period)
    {
        if (!_sample_timer.begin(_sampleISR, this_period * 1000))
        {
            Serial.println("Failed to start sample timer");
            _running = false;
            return;
        }

        last_period = this_period;
    }

    _running = true;
}

void logger_stopSampling()
{
    _running = false;
    _sample_timer.end();
}

bool logger_getState()
{
    return _running;
}

void logger_serviceBuffer()
{
    /* 14 * 1       timestamp
       7  * 7       mpu floats (accel[3], gyro[3], temp)
       4  * 16      adc readings (up to LOGGER_MAX_ADC_CHANNELS)
       1  * 18      comma separators
     = ~128 bytes per csv row (~4.5 MB/hr @ 10 hz, max 7 MB/hr) */

    if (_tail == _head)
        return;

    char row_buf[CSV_ROW_BUF_LEN];

    // Add timestamp and MPU data to string
    uint16_t len = snprintf(row_buf, CSV_ROW_BUF_LEN,
    "%10lu.%03d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
    _tail->time, _tail->millis,
    _tail->mpu_accel[0], _tail->mpu_accel[1], _tail->mpu_accel[2],
    _tail->mpu_gyro[0], _tail->mpu_gyro[1], _tail->mpu_gyro[2],
    _tail->mpu_temp);

    // Add each of the configured channels
    uint8_t count = (uint8_t) storage_configGetNum(CONFIG_CHANNEL_TOP) -
                    (uint8_t) storage_configGetNum(CONFIG_CHANNEL_BOT);
    for (uint8_t i = 0; i < count; i++)
        len += snprintf(row_buf + len, CSV_ROW_BUF_LEN - len, ",%d", _tail->adc_data[i]);
    
    len += snprintf(row_buf + len, CSV_ROW_BUF_LEN - len, "\r\n");

    // Write to the SD and increment circular buffer if successful
    if (storage_addToLogFile(row_buf, len))
        _tail = ((_tail - _circ_buf) + 1 < CIRC_BUF_LEN) ? _tail + 1 : _circ_buf;
}

void _sampleISR()
{
    uint8_t bottom = (uint8_t) storage_configGetNum(CONFIG_CHANNEL_BOT);
    uint8_t top = (uint8_t) storage_configGetNum(CONFIG_CHANNEL_TOP);
    for (uint8_t i = 0; i < (top - bottom) + 1; i++)
    {
        _head->adc_data[i] = bottom + i;
    }

    // Collect data and a timestamp
    adc_sample(_head->adc_data, (top - bottom) + 1);
    mpu_sample(_head->mpu_accel, _head->mpu_gyro, &_head->mpu_temp);
    _head->time = clock_getLocalNowSeconds();
    _head->millis = clock_millis();

    // Increment write head
    _head = ((_head - _circ_buf) + 1 < CIRC_BUF_LEN) ? _head + 1 : _circ_buf;

    // Update sample timer period if still running
    if (_running) logger_startSampling();
}
