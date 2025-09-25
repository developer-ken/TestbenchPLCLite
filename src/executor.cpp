#include <Arduino.h>
#include <BlocklyInterpreter.h>
#include <DefaultHandlers.h>
#include <SD.h>
#include <StreamUtils.h>
#include "display.h"
#include "defs.h"
#include "pinout.h"

BlocklyInterpreter blockly;
JsonDocument _json;

// IO_OUT
int _A_IO_OUT(JsonObject jb, BlocklyInterpreter *b)
{
    char pinid = jb["fields"]["ID"].as<const char *>()[2];
    int value = b->eval(jb["inputs"]["VALUE"]);
    switch (pinid)
    {
    case '1':
        digitalWrite(PIN_DO1, value ? HIGH : LOW);
        break;
    case '2':
        digitalWrite(PIN_DO2, value ? HIGH : LOW);
        break;
    case '3':
        digitalWrite(PIN_DO3, value ? HIGH : LOW);
        break;
    case '4':
        digitalWrite(PIN_DO4, value ? HIGH : LOW);
        break;
    default:
        return false;
    }
    return true;
}

// DI_IN
int _V_DI_IN(JsonObject jb, BlocklyInterpreter *b)
{
    char pinid = jb["fields"]["ID"].as<const char *>()[2];
    switch (pinid)
    {
    case '1':
        return digitalRead(PIN_DI1) == HIGH ? 1 : 0;
    case '2':
        return digitalRead(PIN_DI2) == HIGH ? 1 : 0;
    case '3':
        return digitalRead(PIN_DI3) == HIGH ? 1 : 0;
    case '4':
        return digitalRead(PIN_DI4) == HIGH ? 1 : 0;
    default:
        return 0;
    }
}

// AI_IN
int _V_AI_IN(JsonObject jb, BlocklyInterpreter *b)
{
    char pinid = jb["fields"]["ID"].as<const char *>()[2];
    switch (pinid)
    {
    case '1':
        return analogRead(PIN_AI1);
    case '2':
        return analogRead(PIN_AI2);
    default:
        return 0;
    }
}

// CTRL_DELAY
int _A_CTRL_DELAY(JsonObject jb, BlocklyInterpreter *b)
{
    int ms = b->eval(jb["inputs"]["NAME"]);
    for (int i = 0; (i < ms && !b->_flag_stop); i++)
        delay(1);
    return true;
}

void execRegExtraHandlers()
{
    blockly.registerHandler("EVENT_PWRON", _E_COMMON_NOP);
    blockly.registerHandler("IO_OUT", _A_IO_OUT);
    blockly.registerHandler("DI_IN", _V_DI_IN);
    blockly.registerHandler("AI_IN", _V_AI_IN);
    blockly.registerHandler("CTRL_DELAY", _A_CTRL_DELAY);
}

void RunFileAsync(String filename)
{
    log_i("call RunFileAsync(%s)", filename);
    File file = SD.open(filename);
    log_i("fopen FI");
    if (file)
    {
        log_i("%s", file.readString().c_str());
        file.seek(0);
        if (file.size() < 10)
        {
            log_e("File is too small, possibly a corrupted file");
            file.close();
            return;
        }
        ReadBufferingStream bufferedFile{file, 64};
        log_i("start deserilize...");
        DeserializationError error = deserializeJson(_json, bufferedFile, DeserializationOption::NestingLimit(100));
        file.close();
        if (error)
        {
            log_e("deserializeJson() failed: %s", error.c_str());
            return;
        }
        log_i("Creating new task...");
        // 创建新任务执行选中的代码，防止阻塞，优先级为5
        xTaskCreate(
            [](void *param)
            {
                blockly.exec(_json.as<JsonObject>());
                log_i("Trigger pwr-on event");
                Trigger("EVENT_PWRON");
                vTaskDelete(NULL); // 任务完成后自动删除
            },
            "runfile_task",
            8192,
            NULL,
            5,
            NULL);
    }
    else
    {
        log_e("Failed to open file: %s", filename.c_str());
        return;
    }
}

void Reset()
{
    blockly.clearHandlers();
    if (blockly.isBusy())
    {
        blockly.killAll();
    }
    RegisterDefaultHandlers(&blockly);
    execRegExtraHandlers();

    digitalWrite(PIN_DO1, LOW);
    digitalWrite(PIN_DO2, LOW);
    digitalWrite(PIN_DO3, LOW);
    digitalWrite(PIN_DO4, LOW);
}

void Trigger(String name)
{
    // 创建一个新任务来执行选中的代码，防止阻塞，优先级为5
    xTaskCreate(
        [](void *parameter)
        {
            String *name = static_cast<String *>(parameter);
            blockly.triggerEntrance((*name).c_str());
            delete name;       // 释放内存
            vTaskDelete(NULL); // 删除任务
        },
        "TriggerTask",
        32768,            // 栈大小
        new String(name), // 传递参数
        5,                // 优先级为5
        NULL);
}