/* 
 * File:    console.h
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-24
 * Desc:    Serial-based command line interface module.
 */

#pragma once

#include "adc.h"
#include "mpu.h"
#include "clock.h"

// Function pointer for individual command handler
typedef bool (*console_handler_t)(uint8_t argc, char* argv[]);

// Command structy containg command string and handler
typedef struct console_command_t
{
    const char* command;
    console_handler_t handler;
} console_command_t;

// Table of all implemeted commands
const console_command_t command_table[] =
{
    { "mpu", mpu_console },
    { "adc", adc_console },
    { "clock", clock_console }
};

/*
 * Name:    console_init
 * Desc:    Initialize USB-serial communications
 */
void console_init();

/*
 * Name:    console_tick
 *  unused: unused callback argument
 *  return: true
 * Desc:    Processes received characters. When a newline is received, the
 *            inputted command is looked up in the command table and the 
 *            corresponding handler is called, if found.
 */
bool console_tick(void* unused);
