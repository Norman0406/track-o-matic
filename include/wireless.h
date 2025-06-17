#pragma once

#include <functional>
#include <WiFiMulti.h>
#include <thread>

#include "configuration.h"

class Wireless
{
public:
    using IsConnectedHandler = std::function<void(bool)>;

    Wireless(const Configuration &configuration);
    ~Wireless() = default;

    void setIsConnectedHandler(IsConnectedHandler handler);

    void process();

private:
    const uint32_t connectTimeoutMs_ = 10000;

    IsConnectedHandler isConnectedHandler_;
    WiFiMulti wifi_;

    bool wifiConnected_{false};
};
