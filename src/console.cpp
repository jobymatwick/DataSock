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
    char* cursor = command;
    bool  found  = false;
    
    for (uint8_t i = 0; i < (sizeof(command_table) / sizeof(console_command_t)); i++)
    {
        uint8_t len = strlen(command_table[i].command);
        if (!strncmp(command, command_table[i].command, len) &&
            (*(command + len) == ' ' || *(command + len) == '\0'))
        {
            cursor += (*(command + len) == ' ') ? len + 1 : len;
            if (!command_table[i].handler(cursor))
                Serial.println("Command error!");

            found = true;
            break;
        }
    }
    
    if (!found)
    {
        Serial.printf("Command \"%s\" not found.\r\n", command);
    }
}
