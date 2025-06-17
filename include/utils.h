#pragma once

#include <chrono>
#include <string>

std::string format_time(std::chrono::time_point<std::chrono::system_clock> time, std::string format, bool utc);
std::string timepoint_to_utc_timestamp(std::chrono::time_point<std::chrono::system_clock> time);
std::chrono::time_point<std::chrono::system_clock> utc_timestamp_to_local_timepoint(std::string utcTimestamp);
void print_timepoint_as_local(const std::chrono::system_clock::time_point &tp);
