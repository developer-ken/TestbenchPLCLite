#include <Arduino.h>
#include <SPI.h>
#include "storage.h"
#include "pinout.h"

void storage_setup()
{
    SPI.begin(PIN_SDCLK, PIN_SDMISO, PIN_SDMOSI);
    while (!SD.begin(PIN_SDCS, SPI, 40000000, "/sd", 5U, true))
    {
        log_e("Card Mount Failed");
        vTaskDelay(500);
    }
}