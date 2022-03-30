/* 
 * File:    clock.cpp
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-30
 * Desc:    Functions to get and set the real time using the Teensy's integrated
 *            real time clock.
 *          NOTE: time is kept in the RTC in UTC timezone.
 */

#include "clock.h"

#include <TimeLib.h>
#include <time.h>
#include <common/FsDateTime.h>

#include "storage.h"

#define SECONDS_PER_HOUR 3600
#define TIME_STRING_BUF_LEN 32

#define RTC_SET_FLAG (*(volatile uint8_t*) 0x4003E000)

/*
 * Name:    _getUtcTime
 *  return: real UTC time in seconds from the Teensy's RTC
 * Desc:    Function to get the time from the RTC hardware
 */
time_t _getUtcTime();

/*
 * Name:    _localToUtc
 *  local_time: local time in seconds
 *  return: UTC time in seconds
 * Desc:    Convert local time seconds into UTC seconds
 */
time_t _localToUtc(time_t local_time);

/*
 * Name:    _UtcToLocal
 *  utc_time: UTC time in seconds
 *  return: local time in seconds
 * Desc:    Convert UTC time seconds into local seconds
 */
time_t _UtcToLocal(time_t utc_time);

/*
 * Name:    _repRead32
 *  location: address to read from
 *  return: Steady value at location
 * Desc:    Repeatedly read from an address until the value is steady. Used to
 *            read from dynamic RTC registers that have a slow ripple-overflow.
 */
uint32_t _repRead32(volatile uint32_t* location);

char _time_string_buf[TIME_STRING_BUF_LEN];

bool clock_init()
{
    setSyncProvider(_getUtcTime);

    // Check to see if time successfully synced with RTC
    if (timeStatus() != timeSet)
    {
        return false;
    }

    return true;
}

uint32_t clock_getLocalNowSeconds()
{
    return _UtcToLocal(now());
}

char* clock_getLocalNowString()
{
    time_t local_time = clock_getLocalNowSeconds();

    snprintf(_time_string_buf, TIME_STRING_BUF_LEN,
             "%04d-%02d-%02dT%02d:%02d:%02d%s%02d:00",
             year(local_time), month(local_time), day(local_time),
             hour(local_time), minute(local_time), second(local_time),
             (((int) storage_configGetNum(CONFIG_TIMEZONE)) >= 0)
                 ? "+" : "", ((int) storage_configGetNum(CONFIG_TIMEZONE)));
    
    return _time_string_buf;
}

bool clock_isSet()
{
    return RTC_SET_FLAG;
}

void clock_set(uint32_t utc_time)
{
    Teensy3Clock.set(utc_time);
    setTime(utc_time);
    RTC_SET_FLAG = 1;
}

void clock_fsStampCallback(uint16_t* date, uint16_t* time)
{
    time_t local_time = clock_getLocalNowSeconds();

    *date = FS_DATE(year(local_time), month(local_time), day(local_time));
    *time = FS_TIME(hour(local_time), minute(local_time), second(local_time));
}

uint16_t clock_millis()
{
    uint32_t prescaler = _repRead32(&RTC_TPR);    

    // Scale ticks @ 32.768KHz to microseconds
    //   source: https://community.nxp.com/thread/378715
    uint32_t micros = (prescaler * (1000000UL / 64) + 16384 / 64) / (32768 / 64);
    uint32_t seconds = _repRead32(&RTC_TSR);

    return ((seconds * 1000) + (micros / 1000)) % 1000; // ms into second
}

time_t clock_localHumanToUtc(uint8_t hr, uint8_t min, uint8_t sec, uint8_t day, uint8_t month, uint16_t yr)
{
    tm time = { 0 };

    time.tm_hour = hr;
    time.tm_min = min;
    time.tm_sec = sec;
    time.tm_mday = day;
    time.tm_mon = month - 1;
    time.tm_year = yr - 1900;

    return _localToUtc(mktime(&time));
}

bool clock_console(uint8_t argc, char* argv[])
{
    if (!strcmp("get", argv[1]))
    {
        Serial.printf("Localtime: %s\r\n", clock_getLocalNowString());
        Serial.printf("UTC Epoch: %d.%03d\r\n", now(), clock_millis());

        return true;
    }

    if (!strcmp("set", argv[1]))
    {
        time_t time = clock_getLocalNowSeconds();
        switch (argc)
        {
          // clock set HH MM SS - Just change time
          case 5:
            time = clock_localHumanToUtc(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), day(time), month(time), year(time));
            break;

          // clock set HH MM SS DD MM YYYY - Change date and time
          case 8:
            time = clock_localHumanToUtc(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
            break;
        
          default:
            Serial.println("Must provide local time and optionally date");
            return false;
        }

        clock_set(time);

        Serial.printf("RTC: %d UTC\r\n", time);
        Serial.printf("Localtime: %s\r\n", clock_getLocalNowString());

        return true;
    }

    if (!strcmp("status", argv[1]))
    {
        Serial.printf("Clock has%sbeen set.\r\n", clock_isSet() ? " " : " not ");

        return true;
    }

    return false;
}

time_t _getUtcTime()
{
    return Teensy3Clock.get();
}


time_t _localToUtc(time_t local_time)
{
    return local_time - (((int) storage_configGetNum(CONFIG_TIMEZONE)) * SECONDS_PER_HOUR);
}

time_t _UtcToLocal(time_t utc_time)
{
    return utc_time + (((int) storage_configGetNum(CONFIG_TIMEZONE)) * SECONDS_PER_HOUR);
}

uint32_t _repRead32(volatile uint32_t* location)
{
    uint32_t sample_a = 0, sample_b = 1;

    while (sample_a != sample_b)
    {
        sample_a = *location;
        sample_b = *location;
    }

    return sample_a;
}
