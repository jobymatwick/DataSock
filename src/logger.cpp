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

#define CIRC_BUF_LEN 5
#define CSV_ROW_BUF_LEN 200
#define SD_RINGBUF_LEN 512 * 100

IntervalTimer _sample_timer;

log_entry_t _circ_buf[CIRC_BUF_LEN];
log_entry_t* _head = _circ_buf;
log_entry_t* _tail = _circ_buf;

void logger_ISR()
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

    // Update sample timer period
    logger_startSampling();
}

void logger_startSampling()
{
    static uint16_t last_period = 0;
    uint16_t this_period = (uint16_t) storage_configGetNum(CONFIG_POLL_RATE);

    if (this_period != last_period)
    {
        if (!_sample_timer.begin(logger_ISR, this_period * 1000))
        {
            Serial.println("Failed to start sample timer");
            return;
        }

        last_period = this_period;
    }
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

    // TODO: Add CSV row to SD buffer and

    // Increment read tail
    _tail = ((_tail - _circ_buf) + 1 < CIRC_BUF_LEN) ? _tail + 1 : _circ_buf;
}
