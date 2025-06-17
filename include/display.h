#pragma once

#include <lvgl.h>
#include <string>
#include <functional>
#include <unordered_map>
#include "types.h"

struct Project
{
    ProjectId id;
    std::string name;
    std::string client;
};

struct ActiveTimeEntry
{
    TimeEntryId id;
    std::optional<ProjectId> projectId;
    std::string startedAt;
};

class Display
{
public:
    using StartCallback = std::function<void(std::optional<ProjectId>)>;
    using StopCallback = std::function<void(TimeEntryId)>;
    using RefreshCallback = std::function<void()>;

    Display();
    ~Display() = default;

    void setStartCallback(StartCallback callback);
    void setStopCallback(StopCallback callback);
    void setRefreshCallback(RefreshCallback callback);

    void setUsername(std::string);
    void setProjects(const std::vector<Project> &projects);
    void setRefresh(bool active);
    void setWifiConnected(bool connected);

    void setActiveTimeEntry(TimeEntryId timeEntryId,
                            std::optional<ProjectId> projectId,
                            std::string startedAt);
    std::optional<ActiveTimeEntry> getActiveTimeEntry() const;
    void clearActiveTimeEntry();

    void showError(std::string errorMessage);

    void update();

private:
    void setupDisplay();
    void setupTouch();
    void setupStatusBar(lv_obj_t *parent);
    void setupCommandBar(lv_obj_t *parent);
    void setupProjectsList(lv_obj_t *parent);
    void setupStartStopButton(lv_obj_t *parent);

    void updateDate();
    void updateBatteryLevel();
    void updateScreen();

    void registerCallback(lv_obj_t *obj, lv_event_code_t filter, std::function<void(lv_event_t *)> callback);
    static void eventHandlerTrampoline(lv_event_t *event);
    void eventHandler(lv_event_t *event);

    void onRefreshButtonClicked(lv_event_t *event);
    void onStartStopButtonClicked(lv_event_t *event);
    void onProjectClicked(lv_event_t *event, ProjectId projectId);
    void onCloseMessageBox(lv_obj_t *messageBox);

    std::unordered_map<lv_obj_t *, std::function<void(lv_event_t *)>> callbacks_;

    StartCallback startCallback_;
    StopCallback stopCallback_;
    RefreshCallback refreshCallback_;

    bool wifiConnected_{true}; // Default to true so that the initial update can set it to false
    bool refreshActive_{true}; // Default to true so that the initial update can set it to false
    unsigned long lastTime_;
    lv_display_t *display_;
    uint16_t *buffer_;

    // status bar
    lv_obj_t *date_;
    lv_obj_t *refreshIcon_;
    lv_obj_t *batteryLevel_;
    lv_obj_t *wifiStatus_;

    // command bar
    lv_obj_t *username_;
    lv_obj_t *refreshButton_;
    lv_obj_t *refreshButtonLabel_;

    // projects list
    lv_obj_t *projectsList_;

    struct ProjectEntry
    {
        Project project;
        lv_obj_t *listButton;
        lv_obj_t *listLabel;
    };

    std::unordered_map<ProjectId, ProjectEntry> projectListEntries_;

    // running time entry
    std::optional<ActiveTimeEntry> activeTimeEntry_{std::nullopt};
    lv_obj_t *descriptionLabel_;
    lv_obj_t *startedAtLabel_;
    lv_obj_t *startStopButton_;
    lv_obj_t *startStopButtonLabel_;
};
