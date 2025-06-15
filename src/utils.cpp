#include "utils.h"
#include <sstream>
#include <iomanip>

time_t timegm_compat(struct tm *tm)
{
    // calculate days since Unix epoch
    int year = tm->tm_year + 1900;
    int month = tm->tm_mon + 1;
    int day = tm->tm_mday;

    // adjust for months before march to better handle leap years
    if (month < 3)
    {
        month += 12;
        year--;
    }

    // calculate days since epoch using algorithm
    long days = 365L * (year - 1970) + (year - 1969) / 4 - (year - 1901) / 100 + (year - 1601) / 400;
    days += (153 * (month - 3) + 2) / 5 + day - 1;

    // convert to seconds and add time components
    time_t result = days * 86400L + tm->tm_hour * 3600L + tm->tm_min * 60L + tm->tm_sec;

    return result;
}

std::string format_time(std::chrono::system_clock::time_point utc_time, std::string format, bool utc)
{
    time_t time = std::chrono::system_clock::to_time_t(utc_time);

    struct tm tm;
    if (utc)
    {
        gmtime_r(&time, &tm);
    }
    else
    {
        localtime_r(&time, &tm);
    }
    std::ostringstream oss;
    oss << std::put_time(&tm, format.c_str());
    return oss.str();
}

std::string timepoint_to_utc_timestamp(std::chrono::system_clock::time_point time)
{
    std::string timestamp = format_time(time, "%Y-%m-%dT%H:%M:%S", true);

    auto duration_since_epoch = time.time_since_epoch();
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration_since_epoch);
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration_since_epoch - seconds);

    std::ostringstream oss;
    oss << timestamp;
    oss << '.' << std::setw(6) << std::setfill('0') << microseconds.count() << 'Z';
    return oss.str();
}

std::chrono::system_clock::time_point utc_timestamp_to_local_timepoint(std::string utcTimestamp)
{
    std::tm tm = {};
    std::istringstream ss(utcTimestamp);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    if (ss.fail())
    {
        throw std::runtime_error("Failed to parse timestamp: " + utcTimestamp);
    }

    auto timepoint = std::chrono::system_clock::from_time_t(timegm_compat(&tm));

    // Handle microseconds
    size_t dot_pos = utcTimestamp.find('.');
    if (dot_pos != std::string::npos)
    {
        std::string microseconds_str = utcTimestamp.substr(dot_pos + 1, 6);
        int microseconds = std::stoi(microseconds_str);
        timepoint += std::chrono::microseconds(microseconds);
    }

    return timepoint;
}
