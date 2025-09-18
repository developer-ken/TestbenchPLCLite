#include <Arduino.h>
#include <BlocklyInterpreter.h>
#include <DefaultHandlers.h>
#include "display.h"
#include "defs.h"

BlocklyInterpreter blockly;

int RunFile(String filename)
{
    if (!blockly.execFile(filename))
        return false;
    return blockly.triggerEntrance("EVENT_PWRON");
}

void Reset()
{
    blockly.clearHandlers();
    if (blockly.isBusy())
    {
        blockly.killAll();
    }
    RegisterDefaultHandlers(&blockly);
}

void Trigger(String name){
    // 创建一个新任务来执行选中的代码，防止阻塞，优先级为5
    xTaskCreate(
        [](void* parameter) {
            String* name = static_cast<String*>(parameter);
            blockly.triggerEntrance(*name);
            delete name; // 释放内存
            vTaskDelete(NULL); // 删除任务
        },
        "TriggerTask",
        4096, // 栈大小
        new String(name), // 传递参数
        5, // 优先级为5
        NULL
    );
}