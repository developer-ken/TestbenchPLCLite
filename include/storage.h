#ifndef _STORAGE_H_
#define _STORAGE_H_

#include <Arduino.h>
#include <SD.h>
#include <FS.h>

void storage_setup();
extern FS sdcard;

#endif