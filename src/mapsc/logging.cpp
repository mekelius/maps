#include "logging.hh"

#include <iomanip>
#include <cassert>
#include <optional>

#include "logging_options.hh"

namespace Maps {

LogStream LogStream::global{};

namespace {

bool log_check_flag = false;

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
        case LogContext::definition_creation:
            return "during definition creation";
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

} // anonymous namespace

LogOptions::Lock::~Lock() {
    options_->set_loglevel(DEFAULT_LOGLEVEL);
    options_->locked_ = false;
}

LogOptions::Lock::Lock(LogOptions* options): options_(options) {
    options_->locked_ = true;
}

LogLevel LogOptions::get_loglevel() const {
    return get_loglevel(LogContext::no_context);
}

LogLevel LogOptions::get_loglevel(LogContext context) const {
    return loglevels_.at(static_cast<size_t>(context));
}

void LogOptions::set_loglevel(LogLevel loglevel) {
    for (size_t index = 0; index < LOG_CONTEXT_COUNT; index++)
        if (!per_context_loglevels_overridden_.at(index))
            loglevels_.at(index) = loglevel;
}

void LogOptions::set_loglevel(LogContext context, LogLevel loglevel) {
    auto index = static_cast<size_t>(context);
    per_context_loglevels_overridden_.at(index) = true;
    loglevels_.at(index) = loglevel;
}

std::optional<std::unique_ptr<LogOptions::Lock>> LogOptions::get_lock() {
    assert(!locked_);
    if (locked_)
        return std::nullopt;
    
    std::optional<std::unique_ptr<Lock>> lock{};
    return std::unique_ptr<Lock>{new Lock{this}};
}

inline uint line_col_padding(const SourceLocation& location) {
    return 5;

    // TODO
    // return width < global_options.LINE_COL_FORMAT_PADDING ? 
    //     std::string(global_options.LINE_COL_FORMAT_PADDING - width, ' ') : " ";
}

bool logs_since_last_check() {
    bool value = log_check_flag;
    log_check_flag = false;
    return value;
}

std::optional<std::unique_ptr<LogOptions::Lock>> LogStream::lock() {
    return std::move(*options_.get_lock());
}

std::optional<std::unique_ptr<LogOptions::Lock>> LogStream::set_loglevel(LogLevel loglevel) {
    if (options_.is_locked()) {
        LogNoContext::compiler_error(NO_SOURCE_LOCATION) << 
            "Trying to set logoptions while locked, ignoring";
        return std::nullopt;
    }
    options_.set_loglevel(loglevel);

    return options_.get_lock();
}

std::optional<std::unique_ptr<LogOptions::Lock>> LogStream::set_loglevel(LogContext context, LogLevel loglevel) {
    if (options_.is_locked()) {        
        LogNoContext::compiler_error(NO_SOURCE_LOCATION) << 
            "Trying to set logoptions while locked, ignoring";
        return std::nullopt;
    }
    options_.set_loglevel(context, loglevel);

    return options_.get_lock();
}

LogStream& LogStream::begin(LogContext logcontext, LogLevel loglevel, const SourceLocation& location) {
    is_open_ = options_.get_loglevel(logcontext) >= loglevel;

    if (!is_open_)
        return *this;

    log_check_flag = true;

    *options_.ostream << location.line << ':' << location.column 
        << std::setw(line_col_padding(location))
        << "  "
        << prefix(loglevel);
                
    // if (global_options.print_context_prefixes)
    //     *global_options.ostream << context_prefix;

    
    return *this;
}

    

} //namespace Maps