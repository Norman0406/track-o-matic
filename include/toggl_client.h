#pragma once

#include "client_interface.h"

class TogglClient : public ClientInterface
{
public:
    TogglClient(std::string apiToken);
    ~TogglClient() = default;

    JsonDocument getMe() const override;
    JsonDocument getProjects() const override;
    JsonDocument getWorkspaces() const override;
    JsonDocument listClients(WorkspaceId workspaceId) const override;
    JsonDocument startTimeEntry(WorkspaceId workspaceId, std::optional<ProjectId> projectId = std::nullopt) override;
    JsonDocument stopTimeEntry(WorkspaceId workspaceId, TimeEntryId timeEntryId) override;
    JsonDocument getCurrentTimeEntry() const override;

private:
    const std::string apiToken_;
};
