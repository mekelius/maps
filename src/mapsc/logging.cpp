#include "logging.hh"
#include "logging_options.hh"

namespace Maps {

namespace {

constinit LogOptions global_options{};
bool log_check_flag = false;

} // anonymous namespace


LogOptions::Lock LogOptions::Lock::global() {
    return LogOptions::Lock{};
}

LogOptions::Lock::~Lock() {
    global_options.set_loglevel(DEFAULT_LOGLEVEL);
    global_lock_ = false;
}

LogOptions::Lock::Lock() {
    options_ = &global_options;
    global_lock_ = true;
}

LogOptions::Lock LogOptions::set_global(LogLevel loglevel) {
    auto lock = Lock::global();
    lock.options_->set_loglevel(loglevel);
    return lock;
}

LogLevel LogOptions::get_loglevel() const {
    return loglevel_;
}

LogLevel LogOptions::get_loglevel(LogContext context) const {
    return per_context_loglevels.at(static_cast<size_t>(context));
}

void LogOptions::set_loglevel(LogLevel loglevel) {
    for (size_t index = 0; index < LOG_CONTEXT_COUNT; index++) {
        if (!per_context_loglevels_overridden_.at(index))
            per_context_loglevels.at(index) = loglevel;
    }    
    loglevel_ = loglevel;
}

void LogOptions::set_loglevel(LogContext context, LogLevel loglevel) {
    auto index = static_cast<size_t>(context);
    per_context_loglevels_overridden_.at(index) = true;
    per_context_loglevels.at(index) = loglevel;
}


std::string line_col_padding(uint width) {
    return width < global_options.LINE_COL_FORMAT_PADDING ? 
        std::string(global_options.LINE_COL_FORMAT_PADDING - width, ' ') : " ";
}

bool logs_since_last_check() {
    bool value = log_check_flag;
    log_check_flag = false;
    return value;
}

void log_(std::string_view message, LogLevel log_level, LogContext context, 
    std::string_view loglevel_prefix, std::string_view context_prefix, SourceLocation location) {

    if (global_options.get_loglevel(context) < log_level)
        return;

    std::string location_string = location.to_string();
    log_check_flag = true;
    
    *global_options.ostream
        << location_string 
        << line_col_padding(location_string.size())
        << loglevel_prefix; 
        
    if (global_options.print_context_prefixes)
        *global_options.ostream << context_prefix;

    *global_options.ostream << message << "\n";
}

void log_(std::string_view message, LogLevel log_level, std::string_view loglevel_prefix, 
    SourceLocation location) {

    if (global_options.get_loglevel() < log_level)
        return;

    std::string location_string = location.to_string();
    log_check_flag = true;
    
    *global_options.ostream
        << location_string 
        << line_col_padding(location_string.size())
        << loglevel_prefix; 

    *global_options.ostream << message << "\n";
}

} //namespace Maps