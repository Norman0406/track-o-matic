#pragma once
#include <memory>
#include "display.h"
#include "client_interface.h"
#include "types.h"

class Controller
{
public:
    Controller();
    ~Controller() = default;

    void update();

private:
    void refresh();
    void startTimeEntry(std::optional<ProjectId> projectId);
    void stopTimeEntry(TimeEntryId timeEntryId);
    void updateActiveTimeEntry();

    std::unique_ptr<Display> display_;
    std::unique_ptr<ClientInterface> client_;

    WorkspaceId workspaceId_{0};
};
