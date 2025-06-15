#include <M5Unified.h>
#include <WiFi.h>
#include "controller.h"
#include "config.h"

std::unique_ptr<Controller> controller;

const char *ntpServer = "pool.ntp.org";

void setup()
{
    try
    {
        Serial.begin(115200);

        delay(500);

        M5.begin();
        WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());

        log_i("Connecting to WiFi: %s", wifiSSID.c_str());
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
        }
        log_i("WiFi connected, IP address: %s", WiFi.localIP().toString().c_str());

        // update time from NTP
        configTime(0, 0, ntpServer);

        controller = std::make_unique<Controller>();
    }
    catch (const std::exception &e)
    {
        log_e("Error in setup: %s", e.what());
    }
}

void loop()
{
    try
    {
        controller->update();
    }
    catch (const std::exception &e)
    {
        log_e("Error in loop: %s", e.what());
    }
}
