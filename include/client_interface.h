#pragma once
#include <string>
#include <exception>
#include <ArduinoJson.h>
#include "types.h"

class HttpException : public std::exception
{
public:
    HttpException(int code, const std::string &message)
        : code_(code), message_(message) {}

    int getCode() const { return code_; }
    const char *what() const noexcept override { return message_.c_str(); }

private:
    int code_;
    std::string message_;
};

class JsonException : public std::exception
{
public:
    JsonException(String payload, const std::string &message)
        : payload_(payload), message_(message) {}

    String getPayload() const { return payload_; }
    const char *what() const noexcept override { return message_.c_str(); }

private:
    String payload_;
    std::string message_;
};

class ClientInterface
{
public:
    ClientInterface() = default;
    virtual ~ClientInterface() = default;

    virtual JsonDocument getMe() const = 0;
    virtual JsonDocument getProjects() const = 0;
    virtual JsonDocument getWorkspaces() const = 0;
    virtual JsonDocument listClients(WorkspaceId workspaceId) const = 0;
    virtual JsonDocument startTimeEntry(WorkspaceId workspaceId, std::optional<ProjectId> projectId = std::nullopt) = 0;
    virtual JsonDocument stopTimeEntry(WorkspaceId workspaceId, TimeEntryId timeEntryId) = 0;
    virtual JsonDocument getCurrentTimeEntry() const = 0;
};
