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

#include "clock.h"

#define ERASE_SIZE 262144L
#define CONFIG_NAME "config.txt"

config_option_t config_settings[] = 
{
    { "poll_rate",      "100" },
    { "device_name",    "DataSock" },
    { "timezone",       "-8" }
};

bool _sd_open = false;
SdFs _sd;

void _sdError();

bool storage_init()
{
    FsDateTime::setCallback(clock_fsStampCallback);

    if (!_sd.begin(SdioConfig(FIFO_SDIO)))
    {
        Serial.println("Failed to open SD card.");
        _sdError();

        return false;
    }

    _sd_open = true;
    return _sd_open;
}

bool storage_format()
{
    if (_sd_open)
    {
        _sd.end();
        _sd_open = false;
    }

    _sd.cardBegin(SdioConfig(FIFO_SDIO));

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

    Serial.printf("Found %.2f GB SD card\r\n", (sectors * 512E-8) / 10.0);
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
    uint8_t buf[512];
    if (!_sd.card()->readSector(0, buf))
    {
        Serial.println("Failed to read first block");
        _sdError();
        return false;
    }
    Serial.printf("All blocks erased to 0x%02x\r\n", buf[0]);

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

    if (storage_init() || storage_init() || storage_init() || storage_init())
        return storage_configCreate();

    return false;
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
    conf_file.open(CONFIG_NAME, O_RDWR | O_CREAT | O_EXCL);

    for (uint16_t i = 0; i < sizeof(config_settings) / sizeof(config_option_t); i++)
    {
        conf_file.write(config_settings[i].key);
        conf_file.write("=");
        conf_file.write(config_settings[i].default_val);
        conf_file.write("\r\n");
    }
    
    conf_file.close();

    Serial.println("Default \"" CONFIG_NAME "\" created.");
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
        if (!storage_init())
            return false;

        Serial.println("Last Modified    Size (Bytes) Filename");
        _sd.ls(LS_DATE | LS_SIZE);

        return true;
    }

    if (!strcmp("cat", argv[1]))
    {
        if (!storage_init())
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
        if (!storage_init())
            return false;

        return storage_configCreate();
    }

    return false;
}

void _sdError()
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
