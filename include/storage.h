/* 
 * File:    storage.h
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-30
 * Desc:    Functions to read and write to collected data or configurations 
 *            stored on the SD card.
 */

#pragma once

#include <Arduino.h>



/*
 * Name:    storage_format
 *  return: true if format successful
 * Desc:    Perform a full erase and exFAT format of the SD card. Based on SdFat
 *            library example "SdFormatter.ino"
 */
bool storage_format();

/*
 * Name:    storage_console
 *  argc:   number of arguments
 *  argv:   list of arguments
 * Desc:    Storage console command handler
 */
bool storage_console(uint8_t argc, char* argv[]);
