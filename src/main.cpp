#include <M5Unified.h>
#include "configuration.h"
#include "wireless.h"
#include "controller.h"

std::unique_ptr<Configuration> configuration;
std::unique_ptr<Wireless> wireless;
std::unique_ptr<Controller> controller;

void setup()
{
    try
    {
        Serial.begin(115200);

        delay(500);

        M5.begin();

        configuration = std::make_unique<Configuration>();
        controller = std::make_unique<Controller>(*configuration);

        wireless = std::make_unique<Wireless>(*configuration);
        wireless->setIsConnectedHandler(std::bind(&Controller::setWifiConnected, controller.get(), std::placeholders::_1));
        wireless->process();
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
        wireless->process();
    }
    catch (const std::exception &e)
    {
        log_e("Error in loop: %s", e.what());
    }
}
