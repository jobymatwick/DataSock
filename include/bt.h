/* 
 * File:    bt.h
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-03-05
 * Desc:    Handles bluetooth communication between device and the app. Also
 *            provides an interface for working with the HM-10 BLE module.
 */

#pragma once

#include <Arduino.h>

#include "logger.h"

/*
 * Name:    bt_init
 * Desc:    Start the Blueoother serial and flush the recieve buffer
 */
void bt_init();

/*
 * Name:    bt_active
 *  return: true is in live or hisotrical trnafer modes
 * Desc:    Check the state of bluetooth communications with the app
 */
bool bt_active();

/*
 * Name:    bt_tick
 *  return: always true
 * Desc:    collect received characters and dispatch commands
 */
bool bt_tick();

/*
 * Name:    bt_isLive
 *  return: true if in live mode
 * Desc:    check if in live mode
 */
bool bt_isLive();

/*
 * Name:    bt_sendSample
 *  sample: pointer to sample
 * Desc:    Send a single sample as bytes and a '#' delimiter
 */
void bt_sendSample(log_entry_t* sample);

/*
 * Name:    bt_console
 *  argc:   number of arguments
 *  argv:   list of arguments
 * Desc:    Bluetooth console command handler
 */
bool bt_console(uint8_t argc, char* argv[]);
