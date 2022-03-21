/* 
 * File:    storage.h
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-30
 * Desc:    Functions to read and write to collected data or configurations 
 *            stored on the SD card.
 */

#pragma once

#include <Arduino.h>

#include "logger.h"

#define CONFIG_STRING_LEN 32

typedef enum
{
    CONFIG_DEV_NAME = 0,
    CONFIG_POLL_RATE,
    CONFIG_TIMEZONE,
    CONFIG_MPU_ID,
    CONFIG_CHANNEL_BOT,
    CONFIG_CHANNEL_TOP,
    CONFIG_COUNT
} config_keys_t;

/*
 * Name:    storge_init
 *  return: true if successfully communicating with SD card
 * Desc:    Attempt to communicate with SD card and load config
 */
bool storage_init();

/*
 * Name:    storge_start
 *  return: true if successfully communicating with SD card
 * Desc:    Just attempt to communicate with SD card
 */
bool storage_start();

/*
 * Name:    storage_format
 *  return: true if format successful
 * Desc:    Perform a full erase and exFAT format of the SD card. Based on SdFat
 *            library example "SdFormatter.ino"
 */
bool storage_format();

/*
 * Name:    storage_configDefault
 * Desc:    Set the current config values to the defaults.
 */
void storage_loadDefault();

/*
 * Name:    storage_configCreate
 *  return: true if successfully created config file with default values
 * Desc:    Create or replace the "config.txt" file on the SD card with defaults
 */
bool storage_configCreate();

/*
 * Name:    storage_configLoad
 *  return: true if config file successfully loaded
 * Desc:    Read the config file from the SD.
 */
bool storage_configLoad();

/*
 * Name:    storage_configGetNum
 *  option: Config option to get the value of
 *  return: Numeric value of current config value
 * Desc:    Get the numeric value of a configuration option
 */
float storage_configGetNum(config_keys_t option);

/*
 * Name:    storage_configGetNum
 *  option: Config option to get the value of
 *  return: String value of current config value
 * Desc:    Get the string value of a configuration option
 */
char* storage_configGetString(config_keys_t option);

/*
 * Name:    storage_addToLogFile
 *  text:   String to append to current log file
 *  len:    max number of bytes to append
 *  return: True if len bytes were written to file
 * Desc:    Add data to the current logfile. The file is created and opened if
 *            needed and automatically swapped to a new file every hour.
 */
bool storage_addToLogFile(char* text, uint16_t len);

/*
 * Name:    storage_getLogFiles
 *  count:  pointer to variable to store number of logs found
 *  start:  get entries newer than this time
 *  end:    get entiries older than this
 * Desc:    Get a list of log entires within the provided range. If start or end
 *            is 0, return all entires. Array must be freed manually.
 */
uint32_t* storage_getLogFiles(uint16_t* count, uint32_t start, uint32_t end);

/*
 * Name:    storage_getNextSample
 *  time:   epoch of hour file to get next sample from
 *  log:    log entry struct to populate
 *  return: true if a entry was aquired, else false (error, EOF, etc)
 * Desc:    Get the next entry for a given log file.
 */
bool storage_getNextSample(uint32_t time, log_entry_t* log);

/*
 * Name:    storage_console
 *  argc:   number of arguments
 *  argv:   list of arguments
 * Desc:    Storage console command handler
 */
bool storage_console(uint8_t argc, char* argv[]);
