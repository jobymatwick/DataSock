/* 
 * File:    clock.h
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-30
 * Desc:    Functions to get and set the real time using the Teensy's integrated
 *            real time clock.
 */

#pragma once

#include <Arduino.h>

/*
 * Name:    clock_init
 *  return: true if time synced with RTC
 * Desc:    Sync system time with real time clock
 */
bool clock_init();

/*
 * Name:    clock_getLocalNowSeconds
 *  return: local time in seconds since epoch
 * Desc:    Get the local time in seconds since epoch
 */
uint32_t clock_getLocalNowSeconds();

/*
 * Name:    clock_getLocalNowString
 *  return: Pointer to string containing ISO 8601 timestamp
 * Desc:    Get the local time as an ISO 8601 timestamp
 */
char* clock_getLocalNowString();

/*
 * Name:    clock_console
 *  argc:   number of arguments
 *  argv:   list of arguments
 * Desc:    Time console command handler
 */
bool clock_console(uint8_t argc, char* argv[]);
