/* 
 * File:    console.cpp
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-24
 * Desc:    Serial-based command line interface module.
 */

#include "console.h"

#include <Arduino.h>
#include <string.h>

#define BUF_LEN     128
#define MAX_ARGS    24
#define BAUD_RATE   115200

char  console_buffer[BUF_LEN];
char* console_cursor = console_buffer;

/*
 * Name:     _handleCommand
 *  command: command to run handler for
 * Desc:     Lookup and run the handler associated with a provided command
 */
void _handleCommand(char* command);

void console_init()
{
    Serial.begin(BAUD_RATE);
    Serial.println("Starting up!\r\n>");
}

bool console_tick(void* unused)
{
    uint8_t bytes_to_read = Serial.available();

    for (uint8_t i = 0; i < bytes_to_read; i++)
    {
        char byte = Serial.read();

        // Remove previous char and move cursor back when BACKSPACE is pressed
        if (byte == 0x08)
        {
            if (console_cursor - console_buffer)
            {
                Serial.print("\x08 \x08");
                console_cursor--;
            }

            continue;
        }

        // Echo the received character and save to the buffer
        Serial.write(byte);
        *console_cursor++ = byte;
    }

    // Newline == end of command
    if (*(console_cursor - 2) == '\r' && *(console_cursor - 1) == '\n')
    {
        // Null-terminate end of command string
        *(console_cursor - 2) = '\0';

        if (console_cursor - console_buffer > 2)
            _handleCommand(console_buffer);

        console_cursor = console_buffer;
        Serial.print("> ");
    }
    
    return true;
}

void _handleCommand(char* command)
{
    // Remove leading whitespace
    while (isspace(*command)) ++command;

    char*   argv[MAX_ARGS];
    char*   cursor = command;
    uint8_t argc   = 1;
    bool was_space = false;

    // First arg is always the command
    argv[0] = cursor;

    // Walk through the string counting and collecting args
    while (*cursor != '\0')
    {
        if (*cursor == ' ')
        {
            *cursor = '\0';
            was_space = true;
        }
        else if (was_space)
        {
            was_space = false;
            argv[argc] = cursor;
            argc++;
        }

        cursor++;
    }    
    
    // Lookup the command in the command table and run the handler if found
    bool  found  = false;
    for (uint8_t i = 0; i < (sizeof(command_table) / sizeof(console_command_t)); i++)
    {
        if (!strcmp(argv[0], command_table[i].command))
        {
            if (!command_table[i].handler(argc, argv))
                Serial.println("Command error!");

            found = true;
            break;
        }
    }
    
    if (!found)
    {
        Serial.printf("Command \"%s\" not found.\r\n", argv[0]);
    }
}
