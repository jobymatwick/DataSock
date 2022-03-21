/* 
 * File:    bt.cpp
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-03-05
 * Desc:    Handles bluetooth communication between device and the app. Also
 *            provides an interface for working with the HM-10 BLE module.
 */

#include "bt.h"

#include "console.h"
#include "mpu.h"
#include "clock.h"

#define HM_10_SERIAL    Serial1
#define HM_10_BAUDRATE  230400
#define RECV_BUF        128
#define ASCII_BOT       0x20
#define ASCII_TOP       0x7E
#define MAX_ARGS        4
#define FLUSH_RECV      while (HM_10_SERIAL.available()) HM_10_SERIAL.read()
#define ACK_PERIOD      5000

typedef enum
{
    BT_IDLE = 0,
    BT_LIVE,
    BT_XFER
} bt_states_t;

/*
 * Name:    _handleCommand
 *  command: command string to parse and run
 * Desc:    Lookup and run the appropriate protocol function
 */
static void _handleCommand(char* command);

/*
 * BLUETOOTH PROTOCOL FUNCTIONS
 * - See "Foot App Function Spec" on Google Drive for descriptions
 */
static bool _proto_ack(uint8_t argc, char* argv[]);
static bool _proto_mpu(uint8_t argc, char* argv[]);
static bool _proto_rtc(uint8_t argc, char* argv[]);
static bool _proto_live(uint8_t argc, char* argv[]);
static bool _proto_query(uint8_t argc, char* argv[]);

const console_command_t _bt_proto[] =
{
    { "ack", _proto_ack },
    { "mpu", _proto_mpu },
    { "rtc", _proto_rtc },
    { "lon", _proto_live },
    { "loff", _proto_live },
    { "qry", _proto_query }
};

char _recv_buf[RECV_BUF];
bt_states_t _state = BT_IDLE;
uint32_t _last_ack = 0;

void bt_init()
{
    HM_10_SERIAL.begin(HM_10_BAUDRATE);

    FLUSH_RECV;
}

bool bt_active()
{
    return _state;
}

bool bt_tick()
{
    static char* pos = _recv_buf;

    while (HM_10_SERIAL.available())
    {
        char byte = HM_10_SERIAL.read();

        if (byte == '\r')
            continue;

        if (byte == '\n')
        {
            *pos = '\0';
            pos = _recv_buf;
            _handleCommand(_recv_buf);
            continue;
        }

        // Flush if the receive was possibly interrupted by an AT response
        if (byte == 'O' || byte == 'K' || byte == '+')
        {
            *pos = '\0';
            pos = _recv_buf;
            FLUSH_RECV;
            continue;
        }

        if (byte >= ASCII_BOT && byte <= ASCII_TOP)
            *pos++ = byte;
    }

    // Check if ACK not received in time
    if (_state != BT_IDLE)
    {
        if (millis() - _last_ack > ACK_PERIOD)
            _state = BT_IDLE;
    }

    return true;
}

bool bt_isLive()
{
    return _state == BT_LIVE;
}

void bt_sendSample(log_entry_t* sample)
{
    HM_10_SERIAL.write((uint8_t*) sample, sizeof(*sample));
    HM_10_SERIAL.write('#');
}

bool bt_console(uint8_t argc, char* argv[])
{
    if (!strcmp("at", argv[1]))
    {
        FLUSH_RECV;

        if (argc == 2)
            HM_10_SERIAL.print("AT");
        else
            HM_10_SERIAL.printf("%s", argv[2]);

        uint32_t start = millis();

        while (millis() - start < 1000)
        {
            if (HM_10_SERIAL.available())
                Serial.print((char) HM_10_SERIAL.read());
        }
        
        Serial.println();

        return true;
    }

    return false;
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

    bool found = false;
    for (uint8_t i = 0; i < (sizeof(_bt_proto) / sizeof(console_command_t)); i++)
    {
        if (!strcmp(argv[0], _bt_proto[i].command))
        {
            _bt_proto[i].handler(argc, argv);
            found = true;
            break;
        }
    }

    if (!found)
    {
        Serial.printf("Recieved unknown BT command: %s\r\n", argv[0]);
    }
}

static bool _proto_ack(uint8_t argc, char* argv[])
{
    if (_state == BT_IDLE)
    {
        HM_10_SERIAL.print("ok\r\n");
    }

    _last_ack = millis();

    return true;
}

static bool _proto_mpu(uint8_t argc, char* argv[])
{
    HM_10_SERIAL.printf("ok,%d,%d\r\n", mpu_getAccelRange(), mpu_getGyroRange());
    
    return true;
}

static bool _proto_rtc(uint8_t argc, char* argv[])
{
    if (argc == 2)
    {
        uint32_t time = atoi(argv[1]);
        clock_set(time);
        HM_10_SERIAL.print("ok\r\n");
    }

    return true;
}

static bool _proto_live(uint8_t argc, char* argv[])
{
    switch (argv[0][2])
    {
      case 'n':
        _last_ack = millis();
        _state = BT_LIVE;
        break;
      case 'f':
        _state = BT_IDLE;
        HM_10_SERIAL.print("ok\r\n");
        break;
    }

    return true;
}

static bool _proto_query(uint8_t argc, char* argv[])
{
    uint32_t* data;
    uint16_t len = 0;
    uint32_t start = 0, end = 0;

    if (argc == 3)
    {
        start = atoi(argv[1]);
        end = atoi(argv[2]);
    }

    data = storage_getLogFiles(&len, start, end);

    Serial1.printf("ok,%d", len);
    for (int i = 0; i < len; i++)
        Serial1.printf(",%d", data[i]);
    Serial1.printf("\r\n");

    free(data);

    return true;
}
