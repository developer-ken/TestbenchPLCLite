#include <Arduino.h>
#include <esp_heap_caps.h>

#include "Arduino_GFX_Library.h"
#include "pinout.h"
#include "display.h"
#include "knob.h"

uint16_t *dataregisterpool;

uint8_t errcount = 0, last = 1;
uint32_t failtime[15];
uint32_t failcounter[15];
uint32_t secnow = 0;
uint32_t counter = 0;
bool should_be_on = false;

extern Arduino_GFX *gfx;

void setup()
{
    Serial.begin(115200);
    pinMode(PIN_BROWNOUT, INPUT_PULLUP);
    pinMode(PIN_LEDPWR, OUTPUT);
    pinMode(PIN_LEDHB, OUTPUT);
    pinMode(PIN_BEEP, OUTPUT);

    pinMode(PIN_DO1, OUTPUT);
    pinMode(PIN_DO2, OUTPUT);
    pinMode(PIN_DO3, OUTPUT);
    pinMode(PIN_DO4, OUTPUT);

    log_i("Reg DIs...");

    // pinMode(PIN_DI1, INPUT);
    // pinMode(PIN_DI2, INPUT);
    pinMode(PIN_DI3, INPUT);
    pinMode(PIN_DI4, INPUT);

    log_i("DIs DONE");

    digitalWrite(PIN_LEDHB, HIGH);
    digitalWrite(PIN_LEDPWR, LOW);
    while (digitalRead(PIN_BROWNOUT) == LOW)
    {
        log_i("Waiting for power stable...");
        vTaskDelay(100);
        digitalWrite(PIN_LEDPWR, HIGH);
    }
    analogWrite(PIN_BEEP, 0); // Off
    // analogWriteFrequency(4000);

    /*
    // delay(3000);
    log_i("Allocating data registers...");
    dataregisterpool = (uint16_t *)malloc(sizeof(uint16_t) * 524288); // 1MB for data registers
    log_i("Initializing componentes...");
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

    execRegExtraHandlers();
*/
    lvsetup();
    attachInterrupt(PIN_DI4, []()
                    {
                        if(!should_be_on) return;
                        if(errcount <15){
                            failtime[errcount] = secnow;
                            failcounter[errcount] = counter;
                        }
                        errcount++; }, RISING);
    xTaskCreate([](void *param)
                {
                    for (;;)
                    {
                            while(last==errcount)vTaskDelay(10);
                            last = errcount;
                            gfx->fillScreen(0);
                            gfx->setCursor(0, 0);
                            gfx->setTextColor(WHITE);
                            gfx->setTextSize(1);
                            gfx->printf("ErrCount: %d\n", errcount);
                            for (int i = 0; i < errcount; i++)
                            {
                                gfx->printf("Err%02d: %d@%ds\n", i + 1, failcounter[i],failtime[i]);
                            }
                            digitalWrite(PIN_DO1, errcount>0 ? HIGH : LOW);
                    } },
                "ErrorLogger", 4096, NULL, 1, NULL);

    xTaskCreate([](void *param)
                {
                    for (;;)
                    {
                        vTaskDelay(1000);
                        secnow++;
                    } },
                "Timer", 4096, NULL, 1, NULL);
}

void loop()
{
    digitalWrite(PIN_DO2, HIGH);
    vTaskDelay(100);
    if (digitalRead(PIN_DI4) == HIGH)
    {
        if (errcount < 15)
        {
            failtime[errcount] = secnow;
            failcounter[errcount] = counter;
        }
        errcount++;
    }
    should_be_on = true;
    vTaskDelay(10 * 1000); // delay 10sec
    should_be_on = false;
    digitalWrite(PIN_DO2, LOW);
    vTaskDelay(10 * 1000); // delay 10sec
    counter++;
}