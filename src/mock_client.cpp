#include <mock_client.h>
#include <memory>
#include "mock_data/clients.h"
#include "mock_data/current_time_entry.h"
#include "mock_data/me.h"
#include "mock_data/projects.h"
#include "mock_data/start_time_entry.h"
#include "mock_data/stop_time_entry.h"
#include "mock_data/workspaces.h"
#include "types.h"
#include "utils.h"

namespace
{
    const WorkspaceId mockWorkspaceId = 1;
    const std::uint32_t mockUserId = 1;
    const std::string mockUserFullname = "Mock User";

    JsonDocument printAndReturn(const JsonDocument &doc)
    {
        std::string output;
        serializeJson(doc, output);
        log_d("%s", output.c_str());
        return doc;
    }
}

JsonDocument MockClient::getMe() const
{
    JsonDocument doc;
    deserializeJson(doc, mockMe);

    doc["id"] = mockUserId;
    doc["email"] = "fake_email@gmail.com";
    doc["fullname"] = mockUserFullname;
    doc["timezone"] = "Europe/Zurich";
    doc["default_workspace_id"] = mockWorkspaceId;

    return doc;
}

JsonDocument MockClient::getProjects() const
{
    JsonDocument projectDoc;
    deserializeJson(projectDoc, mockProject);

    JsonDocument projectA = projectDoc;
    projectA["id"] = 1;
    projectA["workspace_id"] = mockWorkspaceId;
    projectA["client_id"] = 1;
    projectA["name"] = "Project A";
    projectA["client_name"] = "Client A";
    projectA["wid"] = projectA["workspace_id"];
    projectA["cid"] = projectA["client_id"];

    JsonDocument projectB = projectDoc;
    projectB["id"] = 2;
    projectB["workspace_id"] = mockWorkspaceId;
    projectB["client_id"] = 1;
    projectB["name"] = "Project B";
    projectB["client_name"] = "Client A";
    projectB["wid"] = projectB["workspace_id"];
    projectB["cid"] = projectB["client_id"];

    JsonDocument projectC = projectDoc;
    projectC["id"] = 3;
    projectC["workspace_id"] = mockWorkspaceId;
    projectC["client_id"] = 2;
    projectC["name"] = "Project C";
    projectC["client_name"] = "Client B";
    projectC["wid"] = projectC["workspace_id"];
    projectC["cid"] = projectC["client_id"];

    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(projectA);
    array.add(projectB);
    array.add(projectC);
    return printAndReturn(doc);
}

JsonDocument MockClient::getWorkspaces() const
{
    JsonDocument workspaceDoc;
    deserializeJson(workspaceDoc, mockWorkspace);

    workspaceDoc["id"] = mockWorkspaceId;
    workspaceDoc["organization_id"] = 1;
    workspaceDoc["name"] = mockUserFullname;

    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(workspaceDoc);
    return printAndReturn(doc);
}

JsonDocument MockClient::listClients(WorkspaceId workspaceId) const
{
    JsonDocument clientDoc;
    deserializeJson(clientDoc, mockClient);

    const std::uint32_t totalClientCount = 2;

    JsonDocument clientA = clientDoc;
    clientA["id"] = 1;
    clientA["wid"] = mockWorkspaceId;
    clientA["name"] = "Client A";
    clientA["total_count"] = totalClientCount;

    JsonDocument clientB = clientDoc;
    clientB["id"] = 2;
    clientB["wid"] = mockWorkspaceId;
    clientB["name"] = "Client B";
    clientB["total_count"] = totalClientCount;

    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(clientA);
    array.add(clientB);
    return printAndReturn(doc);
}

JsonDocument MockClient::startTimeEntry(WorkspaceId workspaceId, std::optional<ProjectId> projectId)
{
    JsonDocument doc;
    deserializeJson(doc, mockStartTimeEntry);

    doc["id"] = 1;
    doc["workspace_id"] = workspaceId;
    doc["start"] = timepoint_to_utc_timestamp(std::chrono::system_clock::now());

    if (projectId)
    {
        doc["project_id"] = *projectId;
        doc["pid"] = *projectId;
    }

    doc["user_id"] = mockUserId;
    doc["uid"] = doc["user_id"];
    doc["wid"] = doc["workspace_id"];

    runningTimeEntry_ = doc;

    return printAndReturn(doc);
}

JsonDocument MockClient::stopTimeEntry(WorkspaceId workspaceId, TimeEntryId timeEntryId)
{
    if (!runningTimeEntry_)
    {
        throw HttpException(400, "No time entry is currently running.");
    }

    JsonDocument doc;
    deserializeJson(doc, mockStopTimeEntry);

    runningTimeEntry_ = std::nullopt;

    return printAndReturn(doc);
}

JsonDocument MockClient::getCurrentTimeEntry() const
{
    if (runningTimeEntry_)
    {
        return printAndReturn(*runningTimeEntry_);
    }

    return printAndReturn(JsonDocument());
}
