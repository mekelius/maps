#ifndef __LOGGING_OPTIONS_HH
#define __LOGGING_OPTIONS_HH

/**
 * This header exists to prevent recompilations
 * The definitions are in logging.cpp
 */

#include "mapsc/logging.hh"

namespace Maps {

class LogOptions {
public:
    class Lock {
    public:
        static Lock global();
        ~Lock();        
        LogOptions* options_;

    private:
        Lock();
        bool global_lock_ = false;
    };

    LogLevel get_loglevel() const;
    LogLevel get_loglevel(LogContext context) const;

    // sets the loglevel for all components
    void set_loglevel(LogLevel loglevel);
    void set_loglevel(LogContext context, LogLevel loglevel);

    LogLevels per_context_loglevels = set_all(LogLevel::info);
    std::ostream* ostream = &std::cout;
    uint LINE_COL_FORMAT_PADDING = 8;
    bool print_context_prefixes = false;
    
private:
    LogLevel loglevel_ = LogLevel::info;
    std::array<bool, LOG_CONTEXT_COUNT> per_context_loglevels_overridden_ = 
        init_array<bool, LOG_CONTEXT_COUNT>(false);
};

} // namespace Maps

#endif