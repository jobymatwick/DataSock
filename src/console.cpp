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
#define ASCII_BOT   0x20
#define ASCII_TOP   0x7E

#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL)

char  console_buffer[BUF_LEN];
char* console_cursor = console_buffer;

/*
 * Name:     _handleCommand
 *  command: command to run handler for
 * Desc:     Parse a command and its arguments and run the asso
 */
void _handleCommand(char* command);

void console_init()
{
    Serial.begin(BAUD_RATE);
    Serial.print("Starting up!\r\n> ");
}

bool console_tick(void* unused)
{
    for (uint8_t i = 0; i < Serial.available(); i++)
    {
        char byte = Serial.read();

        // Remove previous char and move cursor back on BACKSPACE or DELETE
        if (byte == 0x08 || byte == 0x7F)
        {
            if (console_cursor - console_buffer)
            {
                Serial.print("\x08 \x08");
                console_cursor--;
            }

            continue;
        }

        // Ignore \n character. Some terminals send \r\n newlines, some just \r
        if (byte == '\n') continue;

        if (byte == '\r')
        {
            // Null-terminate end of command string & send newline
            *console_cursor = '\0';
            Serial.print("\r\n");

            // Handle the command if present
            if (console_cursor - console_buffer)
                _handleCommand(console_buffer);

            // Reset the cursor and print the new prompt
            console_cursor = console_buffer;
            Serial.print("> ");
        }
        
        // Echo the received printable character and save to the buffer
        if (byte >= ASCII_BOT && byte <= ASCII_TOP)
        {
            Serial.write(byte);
            *console_cursor++ = byte;
        }
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

    if (!strcmp(argv[0], "reset")) CPU_RESTART;
    
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
