#include "Log.hpp"
#include <spdlog/common.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

std::shared_ptr<spdlog::logger> e_ConsoleLogger;

void initialize_logging() {
    e_ConsoleLogger = spdlog::default_logger();

    spdlog::set_level(spdlog::level::trace);

    e_ConsoleLogger->set_pattern("[%^%L%$] %v");
}
