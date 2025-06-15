#include "controller.h"
#include <WiFi.h>
#include "utils.h"
#include "timezones.h"

#ifdef USE_MOCK_CLIENT
#include "mock_client.h"
#else
#include "toggl_client.h"
#endif

Controller::Controller()
{
    display_ = std::make_unique<Display>();
    display_->setRefreshCallback(std::bind(&Controller::refresh, this));
    display_->setStartCallback(std::bind(&Controller::startTimeEntry, this, std::placeholders::_1));
    display_->setStopCallback(std::bind(&Controller::stopTimeEntry, this, std::placeholders::_1));

#ifdef USE_MOCK_CLIENT
    client_ = std::make_unique<MockClient>();
#else
    client_ = std::make_unique<TogglClient>();
#endif
}

void Controller::update()
{
    try
    {
        display_->setWifi(WiFi.status() == WL_CONNECTED);
        display_->update();
    }
    catch (const std::exception &e)
    {
        log_e("Error while updating: %s", e.what());
        display_->showError(e.what());
    }
}

void Controller::refresh()
{
    try
    {
        log_i("Refreshing");

        auto me = client_->getMe();
        const char *fullname = me["fullname"].as<const char *>();
        display_->setUsername(fullname);

        workspaceId_ = me["default_workspace_id"].as<WorkspaceId>();
        std::string timezone = me["timezone"].as<const char *>();

        std::string posix_timezone = "GMT0";

        if (iana_to_posix_timezone_map.count(timezone))
        {
            posix_timezone = iana_to_posix_timezone_map.at(timezone);
        }
        else
        {
            log_w("Timezone '%s' not found in predefined list, using default '%s'", timezone.c_str(), posix_timezone.c_str());
        }

        log_i("Setting timezone '%s' (POSIX '%s')", timezone.c_str(), posix_timezone.c_str());
        setenv("TZ", posix_timezone.c_str(), 1);
        tzset();

        log_i("Current timezone: %s", getenv("TZ"));

        auto apiProjects = client_->getProjects();
        std::vector<Project> projectsList;
        for (const auto &apiProject : apiProjects.as<JsonArray>())
        {
            Project project;
            project.id = apiProject["id"].as<ProjectId>();
            project.name = apiProject["name"].as<const char *>();
            project.client = apiProject["client_name"].as<const char *>();
            projectsList.emplace_back(std::move(project));
        }
        display_->setProjects(projectsList);

        updateActiveTimeEntry();
    }
    catch (const std::exception &e)
    {
        log_e("Error while refreshing: %s", e.what());
        display_->showError(e.what());
    }
}

void Controller::startTimeEntry(std::optional<ProjectId> projectId)
{
    try
    {
        log_i("Starting time entry");
        client_->startTimeEntry(workspaceId_, projectId);
        updateActiveTimeEntry();
    }
    catch (const std::exception &e)
    {
        log_e("Error while starting time entry: %s", e.what());
        display_->showError(e.what());
    }
}

void Controller::stopTimeEntry(TimeEntryId timeEntryId)
{
    try
    {
        log_i("Stopping time entry");
        client_->stopTimeEntry(workspaceId_, timeEntryId);
        display_->clearActiveTimeEntry();
    }
    catch (const std::exception &e)
    {
        log_e("Error while stopping time entry: %s", e.what());
        display_->showError(e.what());
    }
}

void Controller::updateActiveTimeEntry()
{
    try
    {
        auto currentTimeEntry = client_->getCurrentTimeEntry();

        if (!currentTimeEntry.isNull())
        {
            workspaceId_ = currentTimeEntry["workspace_id"].as<WorkspaceId>();
            auto projectIdJson = currentTimeEntry["project_id"];

            std::optional<ProjectId> projectId;
            if (!projectIdJson.isNull())
            {
                projectId = projectIdJson.as<ProjectId>();
            }

            auto time_point = utc_timestamp_to_local_timepoint(currentTimeEntry["start"].as<const char *>());
            auto time = format_time(time_point, "%H:%M:%S", false);

            display_->setActiveTimeEntry(currentTimeEntry["id"].as<TimeEntryId>(),
                                         projectId,
                                         time);
        }
        else
        {
            display_->clearActiveTimeEntry();
        }
    }
    catch (const std::exception &e)
    {
        log_e("Error while updating active time entry: %s", e.what());
        display_->showError(e.what());
    }
}
