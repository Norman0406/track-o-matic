#pragma once

#include <string>
#include <vector>

struct WifiConfiguration
{
    std::string ssid;
    std::string password;
};

using WifiConfigurations = std::vector<WifiConfiguration>;

struct TogglConfiguration
{
    std::string apiToken;
};

class Configuration
{
public:
    Configuration();
    ~Configuration() = default;

    const WifiConfigurations &wifiConfiguration() const;
    const TogglConfiguration &togglConfiguration() const;

private:
    void load();

    WifiConfigurations wifiConfigurations_;
    TogglConfiguration togglConfiguration_;
};
