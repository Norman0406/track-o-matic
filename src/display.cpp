#include "display.h"
#include <M5Unified.h>

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_L8))

namespace
{
    const std::string NO_RUNNING_TIME_ENTRY = "No running time entry";
    const std::string NO_PROJECT = "No project";

    void flush_display_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
    {
        uint8_t *pixels = (uint8_t *)px_map;

        log_v("Flushing display: area=(%d, %d, %d, %d)",
              area->x1, area->y1, area->x2, area->y2);

        M5.Display.pushImage(
            area->x1, area->y1,
            area->x2 - area->x1 + 1,
            area->y2 - area->y1 + 1,
            pixels);

        lv_display_flush_ready(disp);
    }

    void read_touchscreen(lv_indev_t *indev, lv_indev_data_t *data)
    {
        lgfx::touch_point_t tp[1];

        int nums = M5.Display.getTouch(tp, 1);

        if (nums <= 0)
        {
            data->state = LV_INDEV_STATE_RELEASED;
            return;
        }

        log_v("Touchscreen read: nums=%d, point=(%d, %d), size=%d, id=%d",
              nums, tp[0].x, tp[0].y, tp[0].size, tp[0].id);

        data->point.x = tp[0].x;
        data->state = LV_INDEV_STATE_PRESSED;
        data->point.y = tp[0].y;
    }

    const char *getBatterySymbol()
    {
        int battery_level = M5.Power.getBatteryLevel();
        auto isCharging = M5.Power.isCharging();

        log_v("Battery level: %d%%", battery_level);

        if (isCharging != M5.Power.is_charging_t::is_discharging)
        {
            log_v("Battery is charging");
            return LV_SYMBOL_CHARGE;
        }
        else if (battery_level < 10)
        {
            log_v("Battery below 10%%");
            return LV_SYMBOL_BATTERY_EMPTY;
        }
        else if (battery_level < 30)
        {
            log_v("Battery below 30%%");
            return LV_SYMBOL_BATTERY_1;
        }
        else if (battery_level < 60)
        {
            log_v("Battery below 60%%");
            return LV_SYMBOL_BATTERY_2;
        }
        else if (battery_level < 90)
        {
            log_v("Battery below 90%%");
            return LV_SYMBOL_BATTERY_3;
        }
        else
        {
            log_v("Battery full");
            return LV_SYMBOL_BATTERY_FULL;
        }
    }

    lv_obj_t *lv_list_get_btn_label(lv_obj_t *btn)
    {
        for (uint32_t i = 0; i < lv_obj_get_child_count(btn); i++)
        {
            lv_obj_t *child = lv_obj_get_child(btn, i);
            if (lv_obj_check_type(child, &lv_label_class))
            {
                return child;
            }
        }

        throw std::runtime_error("Button does not have a label");
    }
}

Display::Display()
    : startCallback_([](std::optional<ProjectId>) {}),
      stopCallback_([](TimeEntryId) {}),
      refreshCallback_([]() {})
{
    M5.Power.begin();
    M5.Display.begin();
    M5.Display.touch()->init();

    setupDisplay();
    setupTouch();

    // the main container uses a column layout
    lv_obj_t *mainContainer = lv_obj_create(lv_screen_active());
    lv_obj_set_size(mainContainer, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(mainContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(mainContainer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    setupStatusBar(mainContainer);
    setupCommandBar(mainContainer);
    setupProjectsList(mainContainer);
    setupStartStopButton(mainContainer);

    lastTime_ = millis();
}

void Display::setStartCallback(StartCallback callback)
{
    startCallback_ = std::move(callback);
}

void Display::setStopCallback(StopCallback callback)
{
    stopCallback_ = std::move(callback);
}

void Display::setRefreshCallback(RefreshCallback callback)
{
    refreshCallback_ = std::move(callback);
}

void Display::setupDisplay()
{
    lv_init();

    const std::uint32_t height = M5.Display.height();
    const std::uint32_t width = M5.Display.width();

    log_i("Display resolution: %ux%u", width, height);

    display_ = lv_display_create(width, height);

    std::uint32_t buffer_size = height * width / 10 * BYTE_PER_PIXEL;
    log_i("Display buffer size: %u bytes", buffer_size);

    buffer_ = new uint16_t[buffer_size];
    lv_display_set_buffers(display_, buffer_, NULL, buffer_size, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display_, flush_display_callback);
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN);
}

void Display::setupTouch()
{
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, read_touchscreen);
}

void Display::setupStatusBar(lv_obj_t *parent)
{
    // status bar container
    lv_obj_t *statusBar = lv_obj_create(parent);
    lv_obj_set_size(statusBar, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(statusBar, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(statusBar, 0, 0);
    lv_obj_set_style_border_side(statusBar, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_align(statusBar, LV_ALIGN_TOP_MID);
    lv_obj_set_flex_align(statusBar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // date
    date_ = lv_label_create(statusBar);
    lv_obj_align(date_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_label_set_text(date_, "<Date>");

    // horizontal spacer
    lv_obj_t *spacer = lv_obj_create(statusBar);
    lv_obj_set_size(spacer, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_bg_opa(spacer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(spacer, LV_OPA_TRANSP, 0);

    // refresh icon
    refreshIcon_ = lv_label_create(statusBar);
    lv_obj_align(refreshIcon_, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_label_set_text(refreshIcon_, LV_SYMBOL_REFRESH);
    setRefresh(false);

    // wifi status icon
    wifiStatus_ = lv_label_create(statusBar);
    lv_obj_align(wifiStatus_, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_label_set_text(wifiStatus_, LV_SYMBOL_WIFI);
    setWifiConnected(false);

    // battery level icon
    batteryLevel_ = lv_label_create(statusBar);
    lv_obj_align(batteryLevel_, LV_ALIGN_TOP_RIGHT, 0, 0);
}

void Display::setupCommandBar(lv_obj_t *parent)
{
    // command bar container
    lv_obj_t *commandBar = lv_obj_create(parent);
    lv_obj_set_size(commandBar, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(commandBar, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(commandBar, 0, 0);
    lv_obj_set_align(commandBar, LV_ALIGN_TOP_MID);
    lv_obj_set_style_border_opa(commandBar, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_align(commandBar, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // username
    username_ = lv_label_create(commandBar);
    lv_obj_align(username_, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_label_set_text(username_, "");

    // horizontal spacer
    lv_obj_t *spacer = lv_obj_create(commandBar);
    lv_obj_set_size(spacer, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(spacer, 1);
    lv_obj_set_style_border_opa(spacer, LV_OPA_TRANSP, 0);

    refreshButton_ = lv_btn_create(commandBar);
    registerCallback(refreshButton_, LV_EVENT_CLICKED, std::bind(&Display::onRefreshButtonClicked, this, std::placeholders::_1));

    refreshButtonLabel_ = lv_label_create(refreshButton_);
    lv_label_set_text(refreshButtonLabel_, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_font(refreshButtonLabel_, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);
    lv_obj_center(refreshButtonLabel_);
}

void Display::setupProjectsList(lv_obj_t *parent)
{
    projectsList_ = lv_list_create(parent);
    lv_obj_set_size(projectsList_, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(projectsList_, 1);
    lv_obj_set_scrollbar_mode(projectsList_, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_center(projectsList_);
}

void Display::setupStartStopButton(lv_obj_t *parent)
{
    lv_obj_t *runningTimeEntryContainer = lv_obj_create(parent);
    lv_obj_set_size(runningTimeEntryContainer, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(runningTimeEntryContainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(runningTimeEntryContainer, 0, 0);
    lv_obj_set_align(runningTimeEntryContainer, LV_ALIGN_CENTER);
    lv_obj_set_style_border_opa(runningTimeEntryContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_align(runningTimeEntryContainer, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *textContainer = lv_obj_create(runningTimeEntryContainer);
    lv_obj_set_size(textContainer, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(textContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_align(textContainer, LV_ALIGN_TOP_MID);
    lv_obj_set_style_pad_all(textContainer, 0, 0);
    lv_obj_set_style_bg_opa(textContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(textContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_width(textContainer, LV_PCT(100));
    lv_obj_set_flex_grow(textContainer, 1);

    descriptionLabel_ = lv_label_create(textContainer);
    lv_obj_align(descriptionLabel_, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_long_mode(descriptionLabel_, LV_LABEL_LONG_DOT);
    lv_obj_set_width(descriptionLabel_, LV_PCT(100));

    startedAtLabel_ = lv_label_create(textContainer);
    lv_obj_align(startedAtLabel_, LV_ALIGN_LEFT_MID, 0, 0);
    lv_label_set_long_mode(startedAtLabel_, LV_LABEL_LONG_DOT);
    lv_obj_set_width(startedAtLabel_, LV_PCT(100));

    startStopButton_ = lv_btn_create(runningTimeEntryContainer);
    lv_obj_set_size(startStopButton_, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_add_flag(startStopButton_, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_align(startStopButton_, LV_ALIGN_BOTTOM_RIGHT);
    registerCallback(startStopButton_, LV_EVENT_VALUE_CHANGED, std::bind(&Display::onStartStopButtonClicked, this, std::placeholders::_1));

    // set as checked, so that clearActiveTimeEntry() can reset it
    lv_obj_clear_state(startStopButton_, LV_STATE_CHECKED);

    startStopButtonLabel_ = lv_label_create(startStopButton_);
    lv_obj_set_style_text_font(startStopButtonLabel_, &lv_font_montserrat_48, 0);
    lv_obj_center(startStopButtonLabel_);

    clearActiveTimeEntry();
}

void Display::updateDate()
{
    // Get the current date and time
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    // Format the date as "YYYY-MM-DD"
    char dateBuffer[11];
    strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", timeinfo);

    // Update the label text if it has changed
    if (strcmp(lv_label_get_text(date_), dateBuffer) != 0)
    {
        log_v("Updating date label to '%s'", dateBuffer);
        lv_label_set_text(date_, dateBuffer);
    }
}

void Display::updateBatteryLevel()
{
    const char *newBatterySymbol = getBatterySymbol();
    if (strcmp(lv_label_get_text(batteryLevel_), newBatterySymbol) != 0)
    {
        log_v("Updating battery level label");
        lv_label_set_text(batteryLevel_, newBatterySymbol);
    }
}

void Display::updateScreen()
{
    unsigned long current_time = millis();
    lv_tick_inc(current_time - lastTime_);
    lastTime_ = current_time;
    lv_timer_handler();
    delay(10);
}

void Display::update()
{
    log_v("Updating display...");

    updateDate();
    updateBatteryLevel();
    updateScreen();

    log_v("Display updated.");
}

void Display::registerCallback(lv_obj_t *obj, lv_event_code_t filter, std::function<void(lv_event_t *)> callback)
{
    log_d("Registering callback for object: %p", obj);
    callbacks_[obj] = std::move(callback);
    lv_obj_add_event_cb(obj, &Display::eventHandlerTrampoline, filter, this);
}

void Display::eventHandlerTrampoline(lv_event_t *event)
{
    Display *display = static_cast<Display *>(lv_event_get_user_data(event));
    display->eventHandler(event);
}

void Display::eventHandler(lv_event_t *event)
{
    lv_obj_t *target = static_cast<lv_obj_t *>(lv_event_get_target(event));
    auto it = callbacks_.find(target);
    if (it != callbacks_.end())
    {
        it->second(event);
    }
    else
    {
        log_e("No callback found for target object: %p", target);
    }
}

void Display::onRefreshButtonClicked(lv_event_t *event)
{
    log_d("Calling refresh callback");
    setRefresh(true);
    refreshCallback_();
    setRefresh(false);
}

void Display::onStartStopButtonClicked(lv_event_t *event)
{
    if (activeTimeEntry_)
    {
        log_d("Calling stop callback");
        stopCallback_(activeTimeEntry_->id);
    }
    else
    {
        log_d("Calling start callback");
        startCallback_(std::nullopt);
    }
}

void Display::onProjectClicked(lv_event_t *event, ProjectId projectId)
{
    log_i("Starting time entry for project '%llu'", projectId);
    startCallback_(projectId);
}

void Display::setUsername(std::string username)
{
    if (strcmp(lv_label_get_text(username_), username.c_str()) != 0)
    {
        log_i("Updating username label to '%s'", username.c_str());
        lv_label_set_text(username_, username.c_str());
    }
}

void Display::setProjects(const std::vector<Project> &projects)
{
    // clear existing projects
    lv_obj_clean(projectsList_);

    for (const auto &entry : projectListEntries_)
    {
        lv_obj_del(entry.second.listButton);
    }
    projectListEntries_.clear();

    for (const auto &project : projects)
    {
        lv_obj_t *button = lv_list_add_btn(projectsList_, NULL, project.name.c_str());
        registerCallback(button, LV_EVENT_CLICKED, std::bind(&Display::onProjectClicked, this, std::placeholders::_1, project.id));

        lv_obj_t *button_label = lv_list_get_btn_label(button);
        lv_label_set_long_mode(button_label, LV_LABEL_LONG_DOT); // doesn't seem to work, it wraps instead
        lv_obj_set_flex_grow(button_label, 1);

        ProjectEntry projectEntry;
        projectEntry.project = project;
        projectEntry.listButton = button;
        projectEntry.listLabel = button_label;
        projectListEntries_[project.id] = projectEntry;
    }
}

void Display::setRefresh(bool active)
{
    if (active != refreshActive_)
    {
        log_i("Setting refresh state to %s", active ? "true" : "false");

        refreshActive_ = active;
        lv_obj_set_style_opa(refreshIcon_, active ? LV_OPA_100 : LV_OPA_0, 0);
    }
}

void Display::setWifiConnected(bool connected)
{
    if (connected != wifiConnected_)
    {
        log_i("Setting WiFi active state to %s", connected ? "true" : "false");

        wifiConnected_ = connected;
        lv_obj_set_style_opa(wifiStatus_, connected ? LV_OPA_100 : LV_OPA_20, 0);

        if (connected)
        {
            setRefresh(true);
            refreshCallback_();
            setRefresh(false);
        }
    }
}

void Display::setActiveTimeEntry(TimeEntryId timeEntryId,
                                 std::optional<ProjectId> projectId,
                                 std::string startedAt)
{
    // if (lv_obj_has_state(startStopButton_, LV_STATE_CHECKED))
    // {
    //     return;
    // }

    clearActiveTimeEntry();

    std::string description = NO_PROJECT;

    if (projectId)
    {
        if (projectListEntries_.find(*projectId) == projectListEntries_.end())
        {
            log_e("Project ID %llu not found in project list", projectId);
            return;
        }

        const ProjectEntry &projectEntry = projectListEntries_[*projectId];

        description = projectEntry.project.name;

        // set active project button background to a light gray
        lv_obj_set_style_bg_color(projectEntry.listButton, lv_color_make(250, 250, 250), LV_PART_MAIN);
    }

    log_i("Setting active time entry '%llu' started at '%s'",
          timeEntryId, startedAt.c_str());

    lv_label_set_text(descriptionLabel_, description.c_str());
    lv_label_set_text(startedAtLabel_, startedAt.c_str());

    lv_obj_clear_state(startStopButton_, LV_STATE_CHECKED);
    lv_label_set_text(startStopButtonLabel_, LV_SYMBOL_STOP);

    ActiveTimeEntry activeTimeEntry;
    activeTimeEntry.id = timeEntryId;
    activeTimeEntry.projectId = projectId;
    activeTimeEntry.startedAt = startedAt;
    activeTimeEntry_ = activeTimeEntry;
}

std::optional<ActiveTimeEntry> Display::getActiveTimeEntry() const
{
    return activeTimeEntry_;
}

void Display::clearActiveTimeEntry()
{
    // if (!lv_obj_has_state(startStopButton_, LV_STATE_CHECKED))
    // {
    //     return;
    // }

    log_i("Clearing active time entry");

    lv_label_set_text(descriptionLabel_, NO_RUNNING_TIME_ENTRY.c_str());
    lv_label_set_text(startedAtLabel_, "");

    lv_obj_clear_state(startStopButton_, LV_STATE_DEFAULT);
    lv_label_set_text(startStopButtonLabel_, LV_SYMBOL_PLAY);

    // reset color on all project buttons
    for (auto &entry : projectListEntries_)
    {
        lv_obj_set_style_bg_color(entry.second.listButton, lv_color_white(), LV_PART_MAIN);
    }

    activeTimeEntry_ = std::nullopt;
}

void Display::showError(std::string errorMessage)
{
    lv_obj_t *messageBox = lv_msgbox_create(NULL);

    lv_msgbox_add_title(messageBox, "Error");
    lv_obj_t *title = lv_msgbox_get_title(messageBox);
    lv_obj_set_style_text_color(title, lv_color_white(), LV_PART_MAIN);
    lv_msgbox_add_text(messageBox, errorMessage.c_str());
    lv_msgbox_add_close_button(messageBox);

    lv_obj_t *closeButton = lv_msgbox_add_footer_button(messageBox, "OK");
    registerCallback(closeButton, LV_EVENT_CLICKED, std::bind(&Display::onCloseMessageBox, this, messageBox));

    // remove dark backdrop
    lv_obj_set_style_bg_opa(lv_obj_get_parent(messageBox), LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(lv_obj_get_parent(messageBox), lv_color_white(), 0);
}

void Display::onCloseMessageBox(lv_obj_t *messageBox)
{
    lv_msgbox_close(messageBox);
}
