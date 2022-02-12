/* 
 * File:    storage.h
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-30
 * Desc:    Functions to read and write to collected data or configurations 
 *            stored on the SD card.
 */

#pragma once

#include <Arduino.h>

#define CONFIG_STRING_LEN 32
typedef struct option_t
{
    const char* key;
    const char* default_val;
    float num_value;
    char str_value[CONFIG_STRING_LEN];
} config_option_t;

extern config_option_t config_settings[];

/*
 * Name:    storge_init
 *  return: true if successfully communicating with SD card
 * Desc:    Attempt to communicate with SD card and begin the connection
 */
bool storage_init();

/*
 * Name:    storage_format
 *  return: true if format successful
 * Desc:    Perform a full erase and exFAT format of the SD card. Based on SdFat
 *            library example "SdFormatter.ino"
 */
bool storage_format();

/*
 * Name:    storage_configCreate
 *  return: true if successfully created config file with default values
 * Desc:    Create or replace the "config.txt" file on the SD card with defaults
 */
bool storage_configCreate();

/*
 * Name:    storage_console
 *  argc:   number of arguments
 *  argv:   list of arguments
 * Desc:    Storage console command handler
 */
bool storage_console(uint8_t argc, char* argv[]);
