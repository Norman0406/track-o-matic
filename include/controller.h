#pragma once

#include <memory>
#include "configuration.h"
#include "display.h"
#include "client_interface.h"
#include "types.h"

class Controller
{
public:
    Controller(const Configuration &configuration);
    ~Controller() = default;

    void setWifiConnected(bool connected);
    void update();

private:
    void refresh();
    void startTimeEntry(std::optional<ProjectId> projectId);
    void stopTimeEntry(TimeEntryId timeEntryId);
    void updateActiveTimeEntry();

    std::unique_ptr<Display> display_;
    std::unique_ptr<ClientInterface> client_;

    WorkspaceId workspaceId_{0};
    const std::string ntpServer_{"pool.ntp.org"};
};
