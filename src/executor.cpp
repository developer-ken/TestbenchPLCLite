#include <Arduino.h>
#include <BlocklyInterpreter.h>
#include <DefaultHandlers.h>
#include <SD.h>
#include <StreamUtils.h>
#include "display.h"
#include "defs.h"

BlocklyInterpreter blockly;
JsonDocument _json;

void execRegExtraHandlers()
{
    blockly.registerHandler("EVENT_PWRON", _E_COMMON_NOP); // 注册默认事件
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
        DeserializationError error = deserializeJson(_json, bufferedFile, DeserializationOption::NestingLimit(20));
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