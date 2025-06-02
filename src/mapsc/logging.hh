#ifndef __LOGGING_HH
#define __LOGGING_HH

#include <memory>
#include <array>
#include <concepts>
#include <string_view>
#include <iostream>

#include "common/array_helpers.hh"
#include "mapsc/source.hh"

namespace Maps {

constexpr auto LOG_CONTEXTS_START_LINE = __LINE__;
enum class LogContext {
    no_context           = 0,
    compiler_init       = 1,
    lexer               = 2,
    layer1              = 3,
    name_resolution     = 4,
    layer2              = 5,
    layer3              = 6,
    layer4              = 7,
    inline_             = 8,
    concretize          = 9,
    ir_gen_init         = 10,
    ir_gen              = 11,
    REPL                = 12,
    dsir_parser         = 13,
    identifier_creation = 14,
    transform_stage     = 15,
    type_checks         = 16,
    type_casts          = 17,
    eval                = 18,
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
    using OStream = std::ostream;

    class Lock {
    public:
        LogOptions* options_;

        ~Lock();
        Lock(Lock&) = delete;
        Lock& operator=(const Lock&) = delete;
        Lock(Lock&&) = default;
        Lock& operator=(Lock&&) = default;

    private:
        Lock(LogOptions* options);
        friend LogOptions;
    };

    bool is_locked() const { return locked_; };

    LogLevel get_loglevel() const;
    LogLevel get_loglevel(LogContext context) const;

    // sets the loglevel for all components
    void set_loglevel(LogLevel loglevel);
    void set_loglevel(LogContext context, LogLevel loglevel);

    LogLevels loglevels_ = set_all(DEFAULT_LOGLEVEL);
    OStream* ostream = &std::cout;
    uint LINE_COL_FORMAT_PADDING = 8;
    bool print_context_prefixes = false;
    
    [[nodiscard]] std::optional<std::unique_ptr<Lock>> get_lock();

private:
    std::array<bool, LOG_CONTEXT_COUNT> per_context_loglevels_overridden_ = 
        init_array<bool, LOG_CONTEXT_COUNT>(false);
    
    bool locked_ = false;
};

class LogStream;

template<typename T>
concept LogsSelf = requires (T t) { t.log_self_to(std::declval<LogStream&>()); };

template<typename T>
concept Loggable = requires (T t) { {t.log_representation()} -> std::convertible_to<std::string_view>; };

template<typename Stream, typename T>
concept Printable = requires (Stream stream, T t) { {stream << t}; };

constexpr char Endl = '\n';
static_assert(Printable<LogOptions::OStream, typeof(Endl)>);

class LogStream {
public:
    static LogStream global;

    LogStream() = default;
    LogStream(LogOptions options): options_(options) {}

    template<class L>
        requires LogsSelf<L>
    LogStream& operator<<(const L& logs_self) {
        if (!is_open_)
            return *this;

        logs_self.log_self_to(*this);
        return *this;
    }

    template<typename L>
        requires Loggable<L>
    LogStream& operator<<(const L& loggable) {
        if (!is_open_)
            return *this;

        *options_.ostream << loggable.log_representation();
        return *this;
    }

    template<typename T>
        requires Printable<LogOptions::OStream, T>
    LogStream& operator<<(T t) {
        if (!is_open_)
            return *this;

        *options_.ostream << t;
        return *this;
    }

    LogStream& begin(LogContext logcontext, LogLevel loglevel, const SourceLocation& location);

    [[nodiscard]] std::optional<std::unique_ptr<LogOptions::Lock>> lock();
    [[nodiscard]] std::optional<std::unique_ptr<LogOptions::Lock>> set_loglevel(LogLevel level);
    [[nodiscard]] std::optional<std::unique_ptr<LogOptions::Lock>> set_loglevel(LogContext context, LogLevel loglevel);

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