#include "configuration.h"
#include "config.h"

Configuration::Configuration()
{
    load();
}

void Configuration::load()
{
    // TODO: This needs to be read from a file
    wifiConfigurations_ = {
        {wifiSSID1, wifiPassword1},
        {wifiSSID2, wifiPassword2},
    };

    togglConfiguration_ = {
        togglApiKey,
    };
}

const WifiConfigurations &Configuration::wifiConfiguration() const
{
    return wifiConfigurations_;
}

const TogglConfiguration &Configuration::togglConfiguration() const
{
    return togglConfiguration_;
}
