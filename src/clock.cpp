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
 * Name:    _localHumanToUtc
 *  return: UTC time in seconds
 * Desc:    Convert human readable local time into UTC seconds
 */
time_t _localHumanToUtc(uint8_t hr, uint8_t min, uint8_t sec, uint8_t day, uint8_t month, uint16_t yr);

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

// Local time offset from UTC in hours
int8_t _time_zone = -8;

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
             (_time_zone >= 0) ? "+" : "", _time_zone);
    
    return _time_string_buf;
}

bool clock_isSet()
{
    return RTC_SET_FLAG;
}

bool clock_console(uint8_t argc, char* argv[])
{
    if (!strcmp("get", argv[1]))
    {
        Serial.printf("Localtime: %s\r\n", clock_getLocalNowString());
        
        return true;
    }

    if (!strcmp("set", argv[1]))
    {
        time_t time = clock_getLocalNowSeconds();
        switch (argc)
        {
          // clock set HH MM SS - Just change time
          case 5:
            time = _localHumanToUtc(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), day(time), month(time), year(time));
            break;

          // clock set HH MM SS DD MM YYYY - Change date and time
          case 8:
            time = _localHumanToUtc(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
            break;
        
          default:
            Serial.println("Must provide local time and optionally date");
            return false;
        }

        Teensy3Clock.set(time);
        setTime(time);
        RTC_SET_FLAG = 1;

        Serial.printf("RTC: %d UTC\r\n", time);
        Serial.printf("Localtime: %s\r\n", clock_getLocalNowString());

        return true;
    }

    if (!strcmp("tz", argv[1]))
    {
        if (argc == 3)
        {
            _time_zone = atoi(argv[2]);
        }

        Serial.printf("Timezone: %s%d\r\n", (_time_zone >= 0) ? "+" : "", _time_zone);

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

time_t _localHumanToUtc(uint8_t hr, uint8_t min, uint8_t sec, uint8_t day, uint8_t month, uint16_t yr)
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

time_t _localToUtc(time_t local_time)
{
    return local_time - (_time_zone * SECONDS_PER_HOUR);
}

time_t _UtcToLocal(time_t utc_time)
{
    return utc_time + (_time_zone * SECONDS_PER_HOUR);
}
