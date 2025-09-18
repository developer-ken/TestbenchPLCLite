#include <Arduino.h>
#include <esp_heap_caps.h>

#include "pinout.h"
#include "display.h"
#include "knob.h"
#include "server.h"
#include "storage.h"

uint16_t *dataregisterpool;

GConfig config = {false, false, false, false, false, 0};

void setup()
{
    pinMode(PIN_BROWNOUT, INPUT_PULLUP);
    pinMode(PIN_LEDPWR, OUTPUT);
    pinMode(PIN_LEDHB, OUTPUT);
    pinMode(PIN_BEEP, OUTPUT);
    digitalWrite(PIN_LEDHB, HIGH);
    digitalWrite(PIN_LEDPWR, LOW);
    while (digitalRead(PIN_BROWNOUT) == LOW)
    {
        vTaskDelay(100);
        digitalWrite(PIN_LEDPWR, HIGH);
    }
    analogWrite(PIN_BEEP, 0); // Off
    analogWriteFrequency(4000);
    Serial.begin(115200);

    // delay(3000);

    dataregisterpool = (uint16_t *)malloc(sizeof(uint16_t) * 524288); // 1MB for data registers
    init_knob();

    SPI.begin(PIN_SDCLK, PIN_SDMISO, PIN_SDMOSI);
    while (!SD.begin(PIN_SDCS, SPI, 40000000, "/sd", 5U, true))
    {
        log_e("Card Mount Failed");
        vTaskDelay(500);
    }
    
    webserver_setup();
    if (SD.exists("/config.bin"))
    {
        // Load config
        log_i("Loading config from binary record...");
        File f = SD.open("/config.bin", FILE_READ);
        f.read((uint8_t *)&config, sizeof(config));
        f.close();
        log_i("CONF_NonVolatile: %d", config.NonVolatile);
        log_i("CONF_StateRecover: %d", config.StateRecover);
        log_i("CONF_AutoBoot: %d", config.AutoBoot);
        log_i("CONF_Wifi: %d", config.Wifi);
    }
    lvsetup();
}

void loop()
{
    lvloop();
    webserver_loop();
    vTaskDelay(0);
}

void SaveConfig()
{
    log_i("Saving config to binary record...");
    File f = SD.open("/config.bin", FILE_WRITE);
    f.write((uint8_t *)&config, sizeof(config));
    f.flush();
    f.close();
}