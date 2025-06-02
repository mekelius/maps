#ifndef __LOGGING_HH
#define __LOGGING_HH

#include <memory>
#include <array>
#include <concepts>
#include <string_view>
#include <iostream>
#include <optional>

#include "common/array_helpers.hh"

namespace Maps {

struct SourceLocation;

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
    definition_creation = 14,
    transform_stage     = 15,
    type_checks         = 16,
    type_casts          = 17,
    eval                = 18,
};
constexpr auto LOG_CONTEXT_COUNT = __LINE__ - LOG_CONTEXTS_START_LINE - 3;

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

class LogStream;

template<typename Stream, typename T>
concept LogsSelf = requires (Stream stream, T t) { t.log_self_to(stream); };

template<typename Stream, typename T>
concept Loggable = requires (Stream stream, T t) { {stream << t.log_representation()}; };

template<typename Stream, typename T>
concept Printable = requires (Stream stream, T t) { {stream << t}; };

class LogStream {
public:
    using InnerStream = std::ostream;

    class Options {
    public:
        static constexpr LogLevel DEFAULT_LOGLEVEL = LogLevel::info;
        static constexpr uint LINE_PADDING = 3;
        static constexpr uint COL_PADDING = 3;
        static constexpr uint LOGLEVEL_PREFIX_PADDING = 10;

        class Lock {
        public:
            Options* options_;

            ~Lock();
            Lock(Lock&) = delete;
            Lock& operator=(const Lock&) = delete;
            Lock(Lock&&) = default;
            Lock& operator=(Lock&&) = default;

        private:
            Lock(Options* options);
            friend Options;
        };

        bool is_locked() const { return locked_; };

        LogLevel get_loglevel() const;
        LogLevel get_loglevel(LogContext context) const;

        // sets the loglevel for all components
        void set_loglevel(LogLevel loglevel);
        void set_loglevel(LogContext context, LogLevel loglevel);

        LogLevels loglevels_ = set_all(DEFAULT_LOGLEVEL);
        InnerStream* inner_stream = &std::cout;
        bool print_context_prefixes = false;
        
        [[nodiscard]] std::optional<std::unique_ptr<Lock>> get_lock();

    private:
        std::array<bool, LOG_CONTEXT_COUNT> per_context_loglevels_overridden_ = 
            init_array<bool, LOG_CONTEXT_COUNT>(false);
        
        bool locked_ = false;
    };

    static LogStream global;

    LogStream() = default;
    LogStream(Options options): options_(options) {}

    template<typename T>
        requires LogsSelf<InnerStream, T>
    LogStream& operator<<(const T& logs_self) {
        if (!is_open_)
            return *this;

        logs_self.log_self_to(*options_.inner_stream);
        return *this;
    }

    template<typename T>
        requires Loggable<InnerStream, T>
    LogStream& operator<<(const T& loggable) {
        if (!is_open_)
            return *this;

        *options_.inner_stream << loggable.log_representation();
        return *this;
    }

    template<typename T>
        requires Printable<InnerStream, T>
    LogStream& operator<<(T t) {
        if (!is_open_)
            return *this;

        *options_.inner_stream << t;
        return *this;
    }

    LogStream& begin(LogContext logcontext, LogLevel loglevel, const SourceLocation& location);

    [[nodiscard]] std::optional<std::unique_ptr<Options::Lock>> lock();
    [[nodiscard]] std::optional<std::unique_ptr<Options::Lock>> set_loglevel(LogLevel level);
    [[nodiscard]] std::optional<std::unique_ptr<Options::Lock>> set_loglevel(LogContext context, LogLevel loglevel);

private:
    Options options_ = {};
    
    bool is_open_ = true;
};

constexpr char Endl = '\n';

static_assert(Printable<LogStream::InnerStream, typeof(Endl)>);


template <LogContext context>
class LogInContext {
public:
    static LogStream& compiler_error(const SourceLocation& location) {
        return LogStream::global.begin(context, LogLevel::compiler_error, location);
    }
    static LogStream& error(const SourceLocation& location) {
        return LogStream::global.begin(context, LogLevel::error, location);
    }
    static LogStream& warning(const SourceLocation& location) {
        return LogStream::global.begin(context, LogLevel::warning, location);
    }
    static LogStream& info(const SourceLocation& location) {
        return LogStream::global.begin(context, LogLevel::info, location);
    }
    static LogStream& debug(const SourceLocation& location) {
        return LogStream::global.begin(context, LogLevel::debug, location);
    }
    static LogStream& debug_extra(const SourceLocation& location) {
        return LogStream::global.begin(context, LogLevel::debug_extra, location);
    }

    LogInContext() = delete;
};

using LogNoContext = LogInContext<LogContext::no_context>;

} // namespace Maps

#endif