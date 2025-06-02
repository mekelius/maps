#ifndef __LOGGING_HH
#define __LOGGING_HH

#include <array>
#include <concepts>
#include <string_view>
#include <iostream>

#include "common/array_helpers.hh"
#include "mapsc/source.hh"

namespace Maps {

constexpr auto LOG_CONTEXTS_START_LINE = __LINE__;
enum class LogContext {
    no_context           = -1,
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
    transform_stage     = 14,
    type_checks         = 15,
    type_casts          = 16,
    eval                = 17,
};
constexpr auto LOG_CONTEXT_COUNT = __LINE__ - LOG_CONTEXTS_START_LINE - 3;

constexpr std::string_view prefix(LogContext context) {
    switch (context) {
        case LogContext::no_context:
            return "";
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
        case LogContext::transform_stage:
            return "in transform stage";
        case LogContext::type_checks:
            return "in type checks";
        case LogContext::type_casts:
            return "in type cast";
        case LogContext::eval:
            return "during compile time evaluation";
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

class LogOptions {
public:
    static constexpr LogLevel DEFAULT_LOGLEVEL = LogLevel::info;

    class Lock {
    public:
        ~Lock();
        LogOptions* options_;

    private:
        Lock(LogOptions* options);
        friend LogOptions;
    };

    LogLevel get_loglevel() const;
    LogLevel get_loglevel(LogContext context) const;

    // sets the loglevel for all components
    void set_loglevel(LogLevel loglevel);
    void set_loglevel(LogContext context, LogLevel loglevel);

    LogLevels loglevels_ = set_all(DEFAULT_LOGLEVEL);
    std::ostream* ostream = &std::cout;
    uint LINE_COL_FORMAT_PADDING = 8;
    bool print_context_prefixes = false;
    
    Lock get_lock();

private:
    std::array<bool, LOG_CONTEXT_COUNT> per_context_loglevels_overridden_ = 
        init_array<bool, LOG_CONTEXT_COUNT>(false);
    
    bool locked_ = false;
};

class LogStream;

template<class T>
concept LogsSelf = requires (T t) { t.log_self_to(std::declval<LogStream&>()); };

template<class T>
concept Loggable = requires (T t) { {t.log_representation()} -> std::convertible_to<std::string_view>; };

class LogStream {
public:
    static LogStream global;

    LogStream() = default;
    LogStream(LogOptions options): options_(options) {}

    template<typename M>
        requires std::convertible_to<M, std::string_view>
    LogStream& operator<<(M message) {
        if (!is_open_)
            return *this;

        *options_.ostream << message;
        return *this;
    }

    template<class L>
        requires LogsSelf<L>
    LogStream& operator<<(const L& logs_self) {
        if (!is_open_)
            return *this;

        logs_self.log_self_to(*this);
        return *this;
    }

    template<class L>
        requires Loggable<L>
    LogStream& operator<<(const L& loggable) {
        if (!is_open_)
            return *this;

        *options_.ostream << loggable.log_representation();
        return *this;
    }

    template<typename T>
        requires requires (T t) { std::to_string(t); }
    LogStream& operator<<(T t) {
        *options_.ostream << std::to_string(t);
        return *this;
    }

    LogStream& begin(LogContext logcontext, LogLevel loglevel, const SourceLocation& location);

    LogOptions::Lock lock();

private:
    LogOptions options_ = {};
    
    bool is_open_ = true;
};

template <LogContext context>
class LogInContext {
public:
    static LogStream& compiler_error(SourceLocation location) {
        return LogStream::global.begin(context, LogLevel::compiler_error, location);
    }
    static LogStream& error(SourceLocation location) {
        return LogStream::global.begin(context, LogLevel::error, location);
    }
    static LogStream& warning(SourceLocation location) {
        return LogStream::global.begin(context, LogLevel::warning, location);
    }
    static LogStream& info(SourceLocation location) {
        return LogStream::global.begin(context, LogLevel::info, location);
    }
    static LogStream& debug(SourceLocation location) {
        return LogStream::global.begin(context, LogLevel::debug, location);
    }
    static LogStream& debug_extra(SourceLocation location) {
        return LogStream::global.begin(context, LogLevel::debug_extra, location);
    }

    LogInContext() = delete;
};

using LogNoContext = LogInContext<LogContext::no_context>;

} // namespace Maps

#endif