#pragma once
#include <string>

const std::string mockWorkspace = R"(
{
    "id": null,
    "organization_id": null,
    "name": null,
    "premium": false,
    "business_ws": false,
    "admin": true,
    "role": "admin",
    "suspended_at": null,
    "server_deleted_at": null,
    "default_hourly_rate": null,
    "rate_last_updated": null,
    "default_currency": "USD",
    "only_admins_may_create_projects": false,
    "only_admins_may_create_tags": false,
    "only_admins_see_team_dashboard": false,
    "projects_billable_by_default": true,
    "projects_private_by_default": true,
    "projects_enforce_billable": false,
    "limit_public_project_data": false,
    "last_modified": "2025-06-14T00:00:00Z",
    "reports_collapse": true,
    "rounding": 0,
    "rounding_minutes": 0,
    "api_token": "mock_api_token",
    "at": "2025-06-14T08:00:27+00:00",
    "logo_url": "https://assets.track.toggl.com/images/workspace.jpg",
    "ical_url": "/ical/workspace_user/mock_ical_url",
    "ical_enabled": true,
    "csv_upload": null,
    "subscription": null,
    "hide_start_end_times": false,
    "disable_timesheet_view": false,
    "disable_approvals": false,
    "disable_expenses": false,
    "working_hours_in_minutes": null,
    "active_project_count": 10
}
)";
