#include "wireless.h"

#include <WiFi.h>

Wireless::Wireless(const Configuration &configuration)
    : isConnectedHandler_([](bool) {})
{
    for (const auto &wifiConfig : configuration.wifiConfiguration())
    {
        const std::string &wifiSSID = wifiConfig.ssid;
        const std::string &wifiPassword = wifiConfig.password;

        log_i("Adding WiFi SSID: %s", wifiSSID.c_str());
        wifi_.addAP(wifiSSID.c_str(), wifiPassword.c_str());
    }
}

void Wireless::setIsConnectedHandler(IsConnectedHandler handler)
{
    isConnectedHandler_ = std::move(handler);
}

void Wireless::process()
{
    if (wifi_.run(connectTimeoutMs_) == WL_CONNECTED && !wifiConnected_)
    {
        log_i("Connected to WiFi %s with IP %s", WiFi.SSID().c_str(), WiFi.localIP().toString());
        wifiConnected_ = true;
        isConnectedHandler_(true);
    }
    else if (WiFi.status() != WL_CONNECTED && wifiConnected_)
    {
        log_i("Disconnected from WiFi");
        wifiConnected_ = false;
        isConnectedHandler_(false);
    }
}
