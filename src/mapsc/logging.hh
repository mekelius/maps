#ifndef __LOGGING_HH
#define __LOGGING_HH

#include <array>
#include <functional>
#include <iostream>
#include <string>

#include "common/array_helpers.hh"
#include "mapsc/source.hh"

namespace Maps {

constexpr auto LOG_CONTEXTS_START_LINE = __LINE__;
enum class LogContext {
    compiler_init       = 0,
    lexer               = 1,
    layer1              = 2,
    name_resolution     = 3,
    layer2              = 4,
    layer3              = 5,
    layer4              = 6,
    inline_             = 7,
    concretize          = 8,
    ir_gen_init         = 9,
    ir_gen              = 10,
    REPL                = 11,
    dsir_parser         = 12,
    identifier_creation = 13,
};
constexpr auto LOG_CONTEXT_COUNT = __LINE__ - LOG_CONTEXTS_START_LINE - 3;

constexpr std::string_view prefix(LogContext context) {
    switch (context) {
        case LogContext::compiler_init:
            return "during compiler initialization: ";
        case LogContext::lexer:
            return "in lexer: ";
        case LogContext::layer1:
            return "in layer1: ";
        case LogContext::name_resolution:
            return "during name resolution: ";
        case LogContext::layer2:
            return "in layer2: ";
        case LogContext::layer3:
            return "in layer3: ";
        case LogContext::layer4:
            return "in layer4: ";
        case LogContext::inline_:
            return "during function inline: ";
        case LogContext::concretize:
            return "during concretize: ";
        case LogContext::ir_gen_init:
            return "during initializing IR generation: ";
        case LogContext::ir_gen:
            return "during IR generation: ";
        case LogContext::REPL:
            return "in REPL: ";
        case LogContext::dsir_parser:
            return "in dsir parser: ";
        case LogContext::identifier_creation:
            return "during identifier creation";
    }
}

enum class LogLevel {
    compiler_error,
    error,
    warning,
    info,
    debug,
    debug_extra
};
using LogLevels = std::array<LogLevel, LOG_CONTEXT_COUNT>;

constexpr LogLevels set_all(LogLevel log_level) {
    LogLevels log_levels{};
    std::fill(log_levels.begin(), log_levels.end(), log_level);
    return log_levels;
}

constexpr std::string_view prefix(LogLevel loglevel) {
    switch (loglevel) {
        case LogLevel::compiler_error:
            return "COMPILER ERROR: ";
        case LogLevel::error:
            return "Error:   ";
        case LogLevel::warning:
            return "Warning: ";
        case LogLevel::info:
            return "Info:    ";
        case LogLevel::debug:
            return "Debug:   ";
        case LogLevel::debug_extra:
            return "Extra:   ";
    }
}

bool logs_since_last_check();

void log_(std::string_view message, LogLevel log_level, LogContext context, 
    std::string_view loglevel_prefix, std::string_view context_prefix, SourceLocation location);
void log_(std::string_view message, LogLevel log_level, std::string_view loglevel_prefix, 
    SourceLocation location);

inline void log(std::string_view message, LogContext context, LogLevel loglevel, SourceLocation location) {
    log_(message, loglevel, context, prefix(loglevel), prefix(context), location);
}
inline void log(std::string_view message, LogLevel loglevel, SourceLocation location) {
    log_(message, loglevel, prefix(loglevel), location);
}

template <LogContext context>
class LogInContext {
public:
    static void on_level(std::string_view message, LogLevel loglevel, SourceLocation location) {
        log(message, context, loglevel, location);
    }

    static void compiler_error(std::string_view message, SourceLocation location) {
        log(message, context, LogLevel::compiler_error, location);
    }
    static void error(std::string_view message, SourceLocation location) {
        log(message, context, LogLevel::error, location);
    }
    static void warning(std::string_view message, SourceLocation location) {
        log(message, context, LogLevel::warning, location);
    }
    static void info(std::string_view message, SourceLocation location) {
        log(message, context, LogLevel::info, location);
    }
    static void debug(std::string_view message, SourceLocation location) {
        log(message, context, LogLevel::debug, location);
    }
    static void debug_extra(std::string_view message, SourceLocation location) {
        log(message, context, LogLevel::debug_extra, location);
    }

    LogInContext() = delete;
};

class LogNoContext {
public:
    static void on_level(std::string_view message, LogLevel loglevel, SourceLocation location) {
        log(message, loglevel, location);
    }

    static void compiler_error(std::string_view message, SourceLocation location) {
        log(message, LogLevel::compiler_error, location);
    }
    static void error(std::string_view message, SourceLocation location) {
        log(message, LogLevel::error, location);
    }
    static void warning(std::string_view message, SourceLocation location) {
        log(message, LogLevel::warning, location);
    }
    static void info(std::string_view message, SourceLocation location) {
        log(message, LogLevel::info, location);
    }
    static void debug(std::string_view message, SourceLocation location) {
        log(message, LogLevel::debug, location);
    }
    static void debug_extra(std::string_view message, SourceLocation location) {
        log(message, LogLevel::debug_extra, location);
    }

    LogNoContext() = delete;
};

} // namespace Maps

#endif