#include <Arduino.h>
#include <esp_heap_caps.h>

#include "pinout.h"
#include "display.h"
#include "knob.h"
#include "server.h"
#include "storage.h"

uint16_t *dataregisterpool;

struct
{
    bool NonVolatile = false,
         AutoBoot = false,
         StateRecover = false,
         Wifi = false;

    char SelectedProgram[64];
} config;

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
    dataregisterpool = (uint16_t *)malloc(sizeof(uint16_t) * 524288); // 1MB for data registers
    init_knob();
    storage_setup();
    webserver_setup();
    if (SD.exists("/config.bin"))
    {
        // Load config
        File f = SD.open("/config.bin", FILE_READ);
        f.read((uint8_t *)&config, sizeof(config));
        f.close();
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
    File f = SD.open("/config.bin", FILE_WRITE);
    f.write((uint8_t *)&config, sizeof(config));
    f.close();
}