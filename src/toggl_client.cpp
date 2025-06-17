#include <toggl_client.h>
#include <memory>
#include <HTTPClient.h>
#include <chrono>
#include "utils.h"

namespace
{
    const std::string togglBaseUrl = "https://api.track.toggl.com/api/v9/";
    const std::string userAgent = "Track-o-Matic/1.0";

    enum class RequestType
    {
        GET,
        PATCH,
        POST,
        PUT
    };

    std::string requestTypetoString(RequestType request)
    {
        switch (request)
        {
        case RequestType::GET:
            return "GET";
        case RequestType::PATCH:
            return "PATCH";
        case RequestType::POST:
            return "POST";
        case RequestType::PUT:
            return "PUT";
        default:
            throw std::invalid_argument("Invalid request type");
        }
    }

    JsonDocument sendRequest(const std::string &apiToken, RequestType requestType, std::string url, JsonDocument payload)
    {
        auto https = std::make_unique<HTTPClient>();
        https->useHTTP10(true);
        https->setUserAgent(userAgent.c_str());
        https->setAuthorization(apiToken.c_str(), "api_token");
        https->setAuthorizationType("Basic");

        WiFiClientSecure client;
        client.setInsecure(); // Not using certificate check while testing

        https->begin(client, (togglBaseUrl + url).c_str());

        std::string requestTypeString = requestTypetoString(requestType);

        std::string payloadString;
        if (!payload.isNull())
        {
            payloadString = payload.as<std::string>();
            log_d("%s -> \"%s\": %s", requestTypeString.c_str(), url.c_str(), payloadString.c_str());
        }
        else
        {
            log_d("%s -> \"%s\"", requestTypeString.c_str(), url.c_str());
        }

        auto responseCode = https->sendRequest(requestTypeString.c_str(), (uint8_t *)payloadString.c_str(), payloadString.length());

        if (responseCode != 200)
        {
            throw HttpException(responseCode, "Failed to send " + requestTypeString + " request: " + std::to_string(responseCode));
        }

        auto returnPayload = https->getString();

        https->end();

        JsonDocument jsonDocument;
        auto error = deserializeJson(jsonDocument, returnPayload);

        if (error)
        {
            throw JsonException(returnPayload, "Failed to parse JSON response");
        }

        log_d("%s <- \"%s\": %s", requestTypeString.c_str(), url.c_str(), returnPayload.c_str());

        return jsonDocument;
    }

    JsonDocument get(const std::string &apiToken, std::string url)
    {
        return sendRequest(apiToken, RequestType::GET, url, JsonDocument());
    }

    JsonDocument patch(const std::string &apiToken, std::string url)
    {
        return sendRequest(apiToken, RequestType::PATCH, url, JsonDocument());
    }

    JsonDocument post(const std::string &apiToken, std::string url, JsonDocument payload = JsonDocument())
    {
        return sendRequest(apiToken, RequestType::POST, url, payload);
    }
}

TogglClient::TogglClient(std::string apiToken)
    : apiToken_(std::move(apiToken))
{
}

JsonDocument TogglClient::getMe() const
{
    // https://engineering.toggl.com/docs/api/me/#response
    return get(apiToken_, "me");
}

JsonDocument TogglClient::getProjects() const
{
    // https://engineering.toggl.com/docs/api/me/#get-projects
    return get(apiToken_, "me/projects");
}

JsonDocument TogglClient::getWorkspaces() const
{
    // https://engineering.toggl.com/docs/api/me/#get-workspaces
    return get(apiToken_, "me/workspaces");
}

JsonDocument TogglClient::listClients(WorkspaceId workspaceId) const
{
    // https://engineering.toggl.com/docs/api/clients/#get-list-clients
    return get(apiToken_, "workspaces/" + std::to_string(workspaceId) + "/clients");
}

JsonDocument TogglClient::startTimeEntry(WorkspaceId workspaceId, std::optional<ProjectId> projectId)
{
    // https://engineering.toggl.com/docs/api/time_entries/#post-timeentries

    JsonDocument payload;
    payload["duration"] = -1; // -1 means the entry is currently running
    payload["created_with"] = userAgent;
    payload["start"] = timepoint_to_utc_timestamp(std::chrono::system_clock::now());
    payload["workspace_id"] = workspaceId;

    if (projectId)
    {
        payload["project_id"] = *projectId;
    }

    return post(apiToken_, "workspaces/" + std::to_string(workspaceId) + "/time_entries", payload);
}

JsonDocument TogglClient::stopTimeEntry(WorkspaceId workspaceId, TimeEntryId timeEntryId)
{
    // https://engineering.toggl.com/docs/api/time_entries/#patch-stop-timeentry
    return patch(apiToken_, "workspaces/" + std::to_string(workspaceId) + "/time_entries/" + std::to_string(timeEntryId) + "/stop");
}

JsonDocument TogglClient::getCurrentTimeEntry() const
{
    // https://engineering.toggl.com/docs/api/time_entries/#get-get-current-time-entry
    return get(apiToken_, "me/time_entries/current");
}
