/* 
 * File:    storage.h
 * Authors: Gary Huang, Yao Li, Joby Matwick, and Jason Zhang
 * Created: 2022-01-30
 * Desc:    Functions to read and write to collected data or configurations 
 *            stored on the SD card.
 */

#include "storage.h"

#include <SdFat.h>
#include <sdios.h>
#include <TimeLib.h>

#include "clock.h"
#include "logger.h"

#define ERASE_SIZE 262144L
#define CONFIG_NAME "config.txt"
#define READ_BUF_SIZE 256

const char* config_keys[] =
{
    "device_name",
    "poll_rate",
    "timezone",
    "mpu_id",
    "channel_bottom",
    "channel_top"
};

const char* config_defaults[] =
{
    "DataSock",
    "100",
    "-7",
    "0",
    "0",
    "12"
};

typedef struct config_val_t
{
    char str_value[CONFIG_STRING_LEN];
    float num_value;
} config_val_t;

config_val_t config_values[CONFIG_COUNT];

bool _sd_open = false;
SdFs _sd;

/*
 * Name:    _sdError
 * Desc:    Print the SD card error message and clean up the connection on error.
 */
static void _sdError();

/*
 * Name:    _str2int
 *  str:    string to convert from
 *  len:    number of characters to read
 * Desc:    Convert n characters into positive integer.
 */
static uint16_t _str2int(const char* str, uint16_t len);

bool storage_init()
{
    if (!storage_start())
    {
        Serial.println("Failed to open SD card.");
        _sdError();
        storage_loadDefault();
    }
    else
    {
        FsDateTime::setCallback(clock_fsStampCallback);
        _sd_open = true;
        _sd_open = storage_configLoad();
    }

    return _sd_open;
}

bool storage_start()
{
    static bool connected = false;

    _sd_open = _sd.begin(SdioConfig(FIFO_SDIO));

    if (!connected && _sd_open)
        Serial.println("SD connected");

    if (connected && !_sd_open)
        Serial.println("SD disconnected");

    connected = _sd_open;
    return _sd_open;
}

bool storage_format()
{
    bool logger = logger_getState();
    if (logger) logger_stopSampling();

    if (_sd_open)
    {
        _sd.end();
        _sd_open = false;
    }

    if (!storage_start())
    {
        Serial.println("Failed to start SD card");
        _sdError();
        return false;
    }

    if (!_sd.card() || _sd.card()->errorCode())
    {
        Serial.println("Failed to init SD card for format");
        _sdError();
        return false;
    }

    uint32_t sectors = _sd.card()->sectorCount();
    if (!sectors)
    {
        Serial.println("Failed to get sector count for format");
        _sdError();
        return false;
    }

    uint8_t buf[512];
    Serial.printf("Found %.2f GB SD card\r\n", (sectors * 512E-8) / 10.0);
    /*
    Serial.print("Starting erase...     ");

    // Erase all SD sectors
    uint32_t first_block = 0;
    uint32_t last_block;
    uint16_t n = 0;
    while (first_block < sectors)
    {
        last_block = (first_block + ERASE_SIZE > sectors)
            ? sectors - 1 : first_block + ERASE_SIZE - 1;

        if (!_sd.card()->erase(first_block, last_block))
        {
            Serial.printf("Failed to erase %d blocks at %d\r\n", last_block - first_block, first_block);
            _sdError();
            return false;
        }

        Serial.printf("\b\b\b\b%3d%%", (++n * 100) / (sectors / ERASE_SIZE));

        first_block += ERASE_SIZE;
    }

    Serial.print(" complete.\r\n");

    // Verify erase
    if (!_sd.card()->readSector(0, buf))
    {
        Serial.println("Failed to read first block");
        _sdError();
        return false;
    }
    Serial.printf("All blocks erased to 0x%02x\r\n", buf[0]);
    */

    // Format as exFAT
    ExFatFormatter exFatFormatter;
    Serial.println("Starting format...");
    if (!exFatFormatter.format(_sd.card(), buf, &Serial))
    {
        Serial.println("Formatting fail");
        _sdError();
        return false;
    }

    _sd.end();

    if (storage_start() || storage_start() || storage_start() || storage_start())
        return storage_configCreate();

    if (logger) logger_startSampling();

    return false;
}

void storage_loadDefault()
{
    // Replace the current configs with defaults
    for (uint8_t i = 0; i < CONFIG_COUNT; i++)
    {
        strcpy(config_values[i].str_value, config_defaults[i]);
        config_values[i].num_value = atof(config_values[i].str_value);
    }
    Serial.printf("Set %d config options to defaults.\r\n", CONFIG_COUNT);
}

bool storage_configCreate()
{
    if (!_sd_open)
    {
        Serial.println("SD not open!");
        return false;
    }

    if (_sd.exists(CONFIG_NAME))
    {
        Serial.println("Removing original config file");
        _sd.remove(CONFIG_NAME);
    }

    FsFile conf_file;
    if (!conf_file.open(CONFIG_NAME, O_RDWR | O_CREAT | O_EXCL))
    {
        _sdError();
        return false;
    }

    for (uint16_t i = 0; i < CONFIG_COUNT; i++)
    {
        conf_file.write(config_keys[i]);
        conf_file.write("=");
        conf_file.write(config_defaults[i]);
        conf_file.write("\r\n");
    }
    
    conf_file.close();

    Serial.println("Default \"" CONFIG_NAME "\" created.");
    return true;
}

bool storage_configLoad()
{
    storage_loadDefault();

    if (!storage_start())
    {
        Serial.println("SD not open!");
        return false;
    }

    if (!_sd.exists(CONFIG_NAME))
        storage_configCreate();

    FsFile conf_file;
    if (!conf_file.open(CONFIG_NAME, O_RDONLY))
    {
        _sdError();
        return false;
    }

    char buf[READ_BUF_SIZE];
    char *key_curs, *val_curs, *end_curs;
    uint8_t match_cnt = 0;

    while (conf_file.available() > 3)
    {
        uint8_t cnt = (conf_file.available() > READ_BUF_SIZE - 1) ? READ_BUF_SIZE - 1 : conf_file.available();
        uint8_t read = conf_file.readBytesUntil('\n', buf, cnt);

        // Walk-off any leading whitespace
        key_curs = buf;
        while (isspace(*key_curs) && key_curs - buf < read) key_curs++;

        // Ignore comment lines (start with '#')
        if (*key_curs == '#') continue;

        // Walk to first character of value string and replace '=' with '\0'
        val_curs = key_curs;
        while (*val_curs != '=' && val_curs - buf < read) val_curs++;
        *(val_curs++) = '\0';

        // Walk to end of value and set null terminator
        end_curs = val_curs;
        while (end_curs - buf < read + 1)
        {
            if (*end_curs == '\r' || *end_curs == '\n') 
            {
                *end_curs = '\0';
                break;
            }
            end_curs++;
        }

        // Abort if key length is 0
        if (!strlen(key_curs)) continue;

        bool found = false;
        for (uint8_t i = 0; i < CONFIG_COUNT; i++)
        {
            if (!strncmp(key_curs, config_keys[i], strlen(config_keys[i])))
            {
                memcpy(config_values[i].str_value, val_curs, end_curs - val_curs);
                config_values[i].num_value = atof(config_values[i].str_value);
                match_cnt++;
                found = true;
                break;
            }
        }

        if (!found)
        {
            Serial.printf("No match for key \"%s\" (%s)\r\n", key_curs, val_curs);
        }
    }        

    Serial.printf("Loaded %d settings from \"" CONFIG_NAME "\".\r\n", match_cnt);
    return match_cnt;
}

float storage_configGetNum(config_keys_t option)
{
    return config_values[option].num_value;
}

char* storage_configGetString(config_keys_t option)
{
    return config_values[option].str_value;
}

bool storage_addToLogFile(char* text, uint16_t len)
{
    static FsFile file;
    static int8_t file_hour = -1;

    if (!storage_start())
        return false;

    uint32_t now = clock_getLocalNowSeconds();
    int8_t this_hour = hour(now);

    static char filename[50];
    snprintf(filename, 50, "%s_%04d-%02d-%02d_%02d.csv",
             storage_configGetString(CONFIG_DEV_NAME),
             year(now), month(now), day(now), hour(now));
    
    // Open new file if onto next hour or first run
    if (this_hour != file_hour || !_sd.exists(filename))
    {
        if (file.isOpen())
            file.close();

        Serial.printf("Starting file \"%s\"...\r\n", filename);
        file_hour = this_hour;
    }

    if (!file.isOpen())
    {
        if (!file.open(filename, O_RDWR | O_CREAT | O_APPEND))
        {
            Serial.println("Failed to open file!");
            return false;
        }
    }

    if (file.write(text, len) != len)
        return false;

    file.flush();
    return true;
}

uint32_t* storage_getLogFiles(uint16_t* count, uint32_t start, uint32_t end)
{
    *count = 0;
    uint16_t arr_size = 4;
    uint32_t* data;
    FsFile dir, file;
    uint8_t name_len = strlen(storage_configGetString(CONFIG_DEV_NAME));

    dir.open("/");
    if (!dir.isDir())
        return nullptr;

    data = (uint32_t*) malloc(arr_size * sizeof(uint32_t));
    dir.rewindDirectory();

    file = dir.openNextFile(O_RDONLY);
    while (file)
    {
        char name[128];
        file.getName(name, 128);

        if (!strncmp(storage_configGetString(CONFIG_DEV_NAME), name, name_len))
        {
            uint8_t hr, day, month;
            uint16_t yr;
            char* curs = name + name_len + 1;

            yr = _str2int(curs, 4);
            curs += 5;
            month = _str2int(curs, 2);
            curs += 3;
            day = _str2int(curs, 2);
            curs += 3;
            hr = _str2int(curs, 2);

            uint32_t time = clock_localHumanToUtc(hr, 0, 0, day, month, yr);

            if (!start || !end || (time >= start && time <= end))
            {
                (*count)++;
                if (*count > arr_size)
                {
                    arr_size *= 2;
                    data = (uint32_t*) realloc(data, arr_size);
                }
                data[*count - 1] = time;
            }
        }

        file = dir.openNextFile(O_RDONLY);
    }

    return data;
}

bool storage_getNextSample(uint32_t time, log_entry_t* log)
{
    static uint32_t last_time = 0;
    static FsFile file;

    time -= 25200;

    if (time != last_time || !file.isOpen())
    {
        char filename[50];
        snprintf(filename, 50, "%s_%04d-%02d-%02d_%02d.csv",
                storage_configGetString(CONFIG_DEV_NAME),
                year(time), month(time), day(time), hour(time));
        
        if (!file.open(filename, O_RDONLY))
            Serial.printf("Failed to open %s\r\n", filename);
    }

    last_time = time;

    if (!file.isOpen())
    {
        Serial.print("File not open");
        return false;
    }

    char line[200];
    file.setTimeout(100);
    file.readBytesUntil('\n', line, 200);

    uint8_t count = sscanf(line, "%lu.%hu,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd,%hd",
        &(log->time), &(log->millis), &(log->mpu_accel[0]), &(log->mpu_accel[1]), &(log->mpu_accel[2]),
        &(log->mpu_gyro[0]), &(log->mpu_gyro[1]), &(log->mpu_gyro[2]), &(log->mpu_temp),
        &(log->adc_data[0]), &(log->adc_data[1]), &(log->adc_data[2]), &(log->adc_data[3]),
        &(log->adc_data[4]), &(log->adc_data[5]), &(log->adc_data[6]), &(log->adc_data[7]),
        &(log->adc_data[8]), &(log->adc_data[9]), &(log->adc_data[10]), &(log->adc_data[11]),
        &(log->adc_data[12]), &(log->adc_data[13]), &(log->adc_data[14]), &(log->adc_data[15]));

    if (count < 9)
    {
        Serial.printf("Failed to parse line\r\n");
        return false;
    }

    return true;
}

bool storage_console(uint8_t argc, char* argv[])
{
    if (!strcmp("init", argv[1]))
    {
        return storage_init();
    }
    
    if (!strcmp("format", argv[1]))
    {       
        return storage_format();
    }

    if (!strcmp("ls", argv[1]))
    {
        if (!storage_start())
            return false;

        Serial.println("Last Modified    Size (Bytes) Filename");
        _sd.ls(LS_DATE | LS_SIZE);

        return true;
    }

    if (!strcmp("cat", argv[1]))
    {
        if (!storage_start())
            return false;

        if (argc < 3)
        {
            Serial.println("Must specify a filename");
            return false;
        }

        char fname[128];
        char* fcurs = fname;
        for (int i = 0; i < argc - 2; i++)
        {
            char* wcurs = argv[i + 2];

            while (*wcurs != '\0') *fcurs++ = *wcurs++;
            *fcurs++ = ' ';
        }
        *(--fcurs) = '\0';

        if (!_sd.exists(fname))
        {
            Serial.printf("File \"%s\" does not exist\r\n", fname);
            return false;
        }

        FsFile cat_file;
        cat_file.open(fname, O_RDONLY);

        char read_buf[128];
        while (cat_file.available())
        {
            size_t read_size = (cat_file.available() > 127) ? 127 : cat_file.available();
            cat_file.readBytes(read_buf, read_size);
            read_buf[read_size] = '\0';

            Serial.print(read_buf);
        }
        Serial.println();

        cat_file.close();
        return true;
    }

    if (!strcmp("default", argv[1]))
    {
        if (!storage_start())
            return false;

        return storage_configCreate();
    }

    if (!strcmp("load", argv[1]))
    {
        return storage_configLoad();
    }

    if (!strcmp("print", argv[1]))
    {
        Serial.println("Config Key       Current Value    Num    Default Value");

        for (uint8_t i = 0; i < CONFIG_COUNT; i++)
        {
            Serial.printf("%-16s %-16s %-6.f %-16s\r\n",
                          config_keys[i],
                          config_values[i].str_value,
                          config_values[i].num_value,
                          config_defaults[i]);
        }
        
        return true;
    }

    if (!strcmp("query", argv[1]))
    {
        uint32_t start = 0, end = 0;
        if (argc == 4)
        {
            start = atoi(argv[2]);
            end = atoi(argv[3]);
        }
        else
            Serial.print("No date range specified - ");

        uint16_t len = 0;
        uint32_t* data = storage_getLogFiles(&len, start, end);
        
        Serial.printf("Found %d log files\r\n", len);

        for (int i = 0; i < len; i++)
            Serial.printf("%s%d", (i == 0) ? "" : ", ", data[i]);

        Serial.println();
        free(data);

        return true;
    }

    if (!strcmp("get", argv[1]))
    {
        if (argc != 3)
            return false;

        uint32_t time = atoi(argv[2]);

        log_entry_t e;
        memset(&e, 0, sizeof(log_entry_t));

        storage_getNextSample(time, &e);
    
        for (uint8_t i = 0; i < sizeof(log_entry_t); i++)
            Serial.printf("0x%02X%s", ((char*) (&e))[i],
                (i == sizeof(log_entry_t) - 1) ? "\r\n" : ", ");

        return true;
    }

    return false;
}

static void _sdError()
{
    _sd_open = false;

    if (!_sd.card())
    {
        Serial.println("Invalid SD config!");
    }
    else if (_sd.card()->errorCode() == SD_CARD_ERROR_CMD0)
    {
        Serial.println("No card, wrong chip select pin, or wiring error?");
    }

    Serial.printf("SD error code: 0x%02X, data: 0x%02X\r\n",
                  _sd.card()->errorCode(), _sd.card()->errorData());
}

static uint16_t _str2int(const char* str, uint16_t len)
{
    int val = 0;
    for(int i = 0; i < len; ++i)
        val = val * 10 + (str[i] - '0');

    return val;
}
