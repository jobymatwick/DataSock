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

SdFs sd;

bool storage_format()
{
    sd.end(); // End SD usage if started

    SdCardFactory factory;
    SdCard* card = factory.newCard(SdioConfig(FIFO_SDIO));
    if (!card || card->errorCode())
    {
        Serial.println("Failed to init SD card for format");
        return false;
    }

    uint32_t sectors = card->sectorCount();
    if (!sectors)
    {
        Serial.println("Failed to get sector count for format");
        return false;
    }

    Serial.printf("Found %.2f GB SD card\r\n", (sectors * 512E-8) / 10.0);
    Serial.print("Starting erase...\r\n");

    // Erase all SD sectors
    uint32_t first_block = 0;
    uint32_t last_block;
    uint8_t n = 0;
    while (first_block < sectors)
    {
        last_block = (first_block + ERASE_SIZE > sectors)
            ? sectors - 1 : first_block + ERASE_SIZE - 1;

        if (!card->erase(first_block, last_block))
        {
            Serial.printf("Failed to erase %d blocks at %d\r\n", last_block - first_block, first_block);
            return false;
        }

        Serial.print(".");
        if ((n++) % 80 == 79) Serial.print("\r\n");

        first_block += ERASE_SIZE;
    }

    // Verify erase
    uint8_t buf[512];
    if (!card->readSector(0, buf))
    {
        Serial.println("\r\nFailed to read first block");
        return false;
    }
    Serial.printf("\r\nAll blocks erased to 0x%02x\r\n", buf[0]);

    // Format as exFAT
    ExFatFormatter exFatFormatter;
    Serial.println("Starting format...");
    if (!exFatFormatter.format(card, buf, &Serial))
    {
        Serial.println("Formatting fail");
        return false;
    }

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
