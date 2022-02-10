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

#define ERASE_SIZE 262144L

SdFs _sd;

void _errorHalt();

bool storage_format()
{
    _sd.cardBegin(SdioConfig(FIFO_SDIO));

    if (!_sd.card() || _sd.card()->errorCode())
    {
        Serial.println("Failed to init SD card for format");
        _errorHalt();
        return false;
    }

    uint32_t sectors = _sd.card()->sectorCount();
    if (!sectors)
    {
        Serial.println("Failed to get sector count for format");
        _errorHalt();
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
            _errorHalt();
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
        _errorHalt();
        return false;
    }
    Serial.printf("All blocks erased to 0x%02x\r\n", buf[0]);

    // Format as exFAT
    ExFatFormatter exFatFormatter;
    Serial.println("Starting format...");
    if (!exFatFormatter.format(_sd.card(), buf, &Serial))
    {
        Serial.println("Formatting fail");
        _errorHalt();
        return false;
    }

    _sd.end();
    return true;
}

bool storage_console(uint8_t argc, char* argv[])
{
    if (!strcmp("format", argv[1]))
    {       
        return storage_format();
    }

    return false;
}

void _errorHalt()
{
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

    _sd.end();
}
