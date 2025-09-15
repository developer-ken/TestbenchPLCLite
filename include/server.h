#ifndef _WEBSERVER_H_
#define _WEBSERVER_H_
#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <FS.h>
#include <LittleFS.h>

extern String ssid;
extern String password;

void webserver_setup();
void webserver_loop();
void WifiEnable();
void WifiDisable();

#endif