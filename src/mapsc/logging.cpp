#include "logging.hh"

#include <iomanip>

#include "logging_options.hh"

namespace Maps {

LogStream LogStream::global{};

namespace {

bool log_check_flag = false;

} // anonymous namespace

LogOptions::Lock::~Lock() {
    options_->set_loglevel(DEFAULT_LOGLEVEL);
    options_->locked_ = false;
}

LogOptions::Lock::Lock(LogOptions* options): options_(options) {
    options_->locked_ = true;
}

LogOptions::Lock LogOptions::set_global(LogContext context, LogLevel loglevel) {
    auto lock = LogStream::global.lock();
    lock.options_->set_loglevel(context, loglevel);
    return lock;
}

LogOptions::Lock LogOptions::set_global(LogLevel loglevel) {
    auto lock = LogStream::global.lock();
    lock.options_->set_loglevel(LogContext::no_context, loglevel);
    return lock;
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

LogOptions::Lock lock();

LogStream& LogStream::begin(LogContext logcontext, LogLevel loglevel, const SourceLocation& location) {
    is_open_ = options_.get_loglevel(logcontext) >= loglevel;

    if (!is_open_)
        return *this;

    log_check_flag = true;

    *options_.ostream << '\n' << location.line << ':' << location.column 
        << std::setw(line_col_padding(location))
        << prefix(loglevel);
                
    // if (global_options.print_context_prefixes)
    //     *global_options.ostream << context_prefix;

    
    return *this;
}

    

} //namespace Maps