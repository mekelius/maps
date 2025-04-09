#include "config.hh"

namespace Logging {

LogLevel LogLevel::quiet;
LogLevel LogLevel::default_;
LogLevel LogLevel::debug;
LogLevel LogLevel::everything;

LogLevel* LogLevel::current = &default_;


void init(LogLevel& log_level) {
    LogLevel::set(log_level);

    LogLevel::quiet.set_message_type(MessageType::general_info, false);

    LogLevel::everything.set_message_type(MessageType::parser_debug, true);
    LogLevel::everything.set_message_type(MessageType::parser_debug_terminals, true);

    LogLevel::debug.set_message_type(MessageType::parser_debug, true);
}

} //namespace logging