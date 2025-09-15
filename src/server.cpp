#include <Arduino.h>
#include <stdlib.h>
#include <time.h>

#include "server.h"
#include "storage.h"
#include "display.h"

WebServer webServer(80);
DNSServer dnsServer;
String ssid = "TestBenchPLC-";
String password = "12345678";
bool enabled = false;

// 生成随机8位密码，包含字母、数字和符号
String generateRandomPassword()
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()";
    const int charsetSize = sizeof(charset) - 1;
    String randomPassword = "";

    // 使用ESP32硬件随机数发生器生成更安全的随机密码
    for (int i = 0; i < 8; i++)
    {
        // 使用esp_random()获取硬件生成的随机数
        // esp_random()返回32位随机数，我们取模以适应字符集大小
        int randomIndex = esp_random() % charsetSize;
        randomPassword += charset[randomIndex];
    }

    return randomPassword;
}

int GetMostFreeChannel()
{
    int n = WiFi.scanNetworks(); // 扫描周围的WiFi网络
    if (n == 0)
    {
        return 1; // 如果没有找到网络，返回默认信道1
    }

    // 初始化每个信道的网络数量计数器
    int channelCounts[14] = {0}; // WiFi信道范围是1-13

    // 统计每个信道上的网络数量
    for (int i = 0; i < n; ++i)
    {
        int channel = WiFi.channel(i);
        if (channel >= 1 && channel <= 13)
        {
            channelCounts[channel - 1]++;
        }
    }

    // 找出网络数量最少的信道
    int minCount = channelCounts[0];
    int minChannel = 1;

    for (int i = 1; i < 13; i++)
    {
        if (channelCounts[i] < minCount)
        {
            minCount = channelCounts[i];
            minChannel = i + 1;
        }
    }

    return minChannel; // 返回最空闲的信道
}

// 获取文件内容的MIME类型
String getContentType(String filename)
{
    if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".svg"))
        return "image/svg+xml";
    else if (filename.endsWith(".json"))
        return "application/json";
    else if (filename.endsWith(".pdf"))
        return "application/pdf";
    else if (filename.endsWith(".zip"))
        return "application/zip";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".woff2"))
        return "font/woff2";
    else if (filename.endsWith(".eot"))
        return "application/vnd.ms-fontobject";
    return "application/octet-stream";
}

void webserver_setup()
{
    LittleFS.begin(false);
    // 获取芯片的MAC地址作为唯一标识
    byte mac[6];
    WiFi.macAddress(mac);

    // 将MAC地址后4位转换为4位十六进制字符串
    String lastFourDigits = "";
    for (int i = 4; i < 6; i++)
    {
        lastFourDigits += String(mac[i], HEX);
    }
    // 转换为大写并确保是4位
    lastFourDigits.toUpperCase();
    if (lastFourDigits.length() > 4)
    {
        lastFourDigits = lastFourDigits.substring(0, 4);
    }
    else if (lastFourDigits.length() < 4)
    {
        while (lastFourDigits.length() < 4)
        {
            lastFourDigits = "0" + lastFourDigits;
        }
    }

    // 创建热点名称，格式为"TestbenchPLC_XXXX"，其中XXXX是芯片序列号后四位
    ssid = "TestbenchPLC_" + lastFourDigits;
    IPAddress local_IP(192, 168, 45, 1);
    IPAddress gateway(192, 168, 45, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);
    WiFi.softAPdisconnect(true);
}

void WifiEnable()
{
    int freechn = 1; // GetMostFreeChannel();
    password = generateRandomPassword();
    WiFi.softAP(ssid.c_str(), password.c_str(), freechn, true, 1, false);
    // WiFi.setTxPower(WIFI_POWER_19_5dBm);
    dnsServer.start(53, "*", WiFi.softAPIP());
    webServer.begin();
    // 加载程序API
    {
        webServer.on("/api/progload", HTTP_GET, []()
                     {
        log_i("/api/progload api request;");
        // 检查是否有name参数
        if (!webServer.hasArg("name")) {
            webServer.send(400, "application/json", "{\"error\":\"Missing name parameter\"}");
            return;
        }
        
        // 获取文件名并确保以.tbp结尾
        String filename = webServer.arg("name");
        if (!filename.endsWith(".tbp")) {
            filename += ".tbp";
        }
        
        // 尝试打开文件
        File file = SD.open("/" + filename, FILE_READ);
        if (!file) {
            webServer.send(404, "application/json", "{\"error\":\"File not found\"}");
            return;
        }
        
        // 读取文件内容
        String content = "";
        while (file.available()) {
            content += (char)file.read();
        }
        file.close();
        
        webServer.send(200, "application/json", content); });
    }

    // 保存程序API
    {
        webServer.on("/api/save", HTTP_POST, []()
                     {
            log_i("/api/save api request;");
            // 检查是否有name参数
            if (!webServer.hasArg("name")) {
                webServer.send(400, "application/json", "{\"error\":\"Missing name parameter\"}");
                return;

        lvproglistupdate();
    }
    
    // 获取文件名并确保以.tbp结尾
    String filename = webServer.arg("name");
    if (!filename.endsWith(".tbp")) {
        filename += ".tbp";
    }
    
    // 检查请求体是否有数据
    if (webServer.hasArg("plain") == false) {
        webServer.send(400, "application/json", "{\"error\":\"No content received\"}");
        return;
    }
    
    // 获取POST数据
    String content = webServer.arg("plain");
    
    // 尝试打开文件进行写入
    File file = SD.open("/" + filename, FILE_WRITE);
    if (!file) {
        webServer.send(500, "application/json", "{\"error\":\"Failed to create file\"}");
        return;
    }
    
    // 写入内容
    size_t bytesWritten = file.print(content);
    file.close();
    
    // 检查是否全部写入成功
    if (bytesWritten != content.length()) {
        webServer.send(500, "application/json", "{\"error\":\"Failed to write complete content\"}");
        return;
    }
    
    // 返回成功响应
    webServer.send(200, "application/json", "{\"status\":\"success\"}"); });
    }

    // 删除程序API
    {
        webServer.on("/api/progdel", HTTP_GET, []()
                     {
            log_i("/api/progdel api request;");
            // 检查是否有name参数
            if (!webServer.hasArg("name")) {
                log_w("Delete request missing name parameter");
                webServer.send(400, "application/json", "{\"error\":\"Missing name parameter\"}");
                return;
            }

            // 获取文件名并确保以.tbp结尾
            String filename = webServer.arg("name");
            if (!filename.endsWith(".tbp")) {
                filename += ".tbp";
            }

            // 尝试删除文件
            if (SD.remove("/" + filename)) {
                log_i("File deleted successfully");
                webServer.send(200, "application/json", "{\"status\":\"success\"}");
            }
            else
            {
                log_w("File deletion failed");
                webServer.send(500, "application/json", "{\"error\":\"Failed to delete file\"}");
            }

            lvproglistupdate(); });
    }

    // 列出程序API
    {
        webServer.on("/api/proglist", HTTP_GET, []()
                     {
            String response = "[";
            File root = SD.open("/");
            if (root) {
                File file = root.openNextFile();
                bool first = true;
                
                while (file) {
                    String fname = file.name();
                    if (!file.isDirectory() && fname.endsWith(".tbp") && fname != "default.tbp") {
                        if (!first) {
                            response += ",";
                        }
                        response += "\"" + fname.substring(0,fname.length()-4) + "\"";
                        first = false;
                    }
                    file = root.openNextFile();
                }
                root.close();
            }
            response += "]";
            webServer.send(200, "application/json", response); });
    }

    // 静态文件请求，从Flash中读取文件
    webServer.onNotFound([]()
                         {
        String path = webServer.uri();
        log_i("Static request for %s", path.c_str());
        // 如果请求的文件存在于LittleFS中，则返回文件
        if (!path.equals("/") && LittleFS.exists(path)) {
            File file = LittleFS.open(path, "r");
            if (file) {
                String contentType = getContentType(path);
                webServer.sendHeader("Cache-Control", "public, max-age=31536000");
                webServer.streamFile(file, contentType);
                file.close();
            } else {
                webServer.send(500, "text/plain", "Could not open file");
            }
        } else {
            // 如果文件不存在，重定向到editor.html
            webServer.sendHeader("Location", "/editor.html");
            webServer.send(302, "text/plain", "Redirecting to editor.html");
        } });
    enabled = true;
}

void WifiDisable()
{
    WiFi.softAPdisconnect(true);
    dnsServer.stop();
    webServer.stop();
    enabled = false;
}

void webserver_loop()
{
    if (!enabled)
        return;
    webServer.handleClient();
    dnsServer.processNextRequest();
}
