#include "logging.hh"

namespace Logging {

LogLevel LogLevel::quiet;
LogLevel LogLevel::default_;
LogLevel LogLevel::debug;
LogLevel LogLevel::everything;

LogLevel* Settings::current_loglevel = &LogLevel::default_;
std::ostream* Settings::ostream;

void LogLevel::init(LogLevel& log_level) {
    Settings::set_loglevel(log_level);

    LogLevel::quiet.set_message_type(MessageType::general_info, false);

    LogLevel::everything.set_message_type(MessageType::parser_debug, true);
    LogLevel::everything.set_message_type(MessageType::parser_debug_terminal, true);

    LogLevel::debug.set_message_type(MessageType::parser_debug, true);
}

void log_error(Location location, const std::string& message) {
    if (!Settings::ostream)
        return;
    if (!Settings::current_loglevel->has_message_type(MessageType::error))
        return;

    *Settings::ostream
        << location.to_string() << line_col_padding(location.to_string().size()) 
        << "error: " << message << "\n";
}

void log_info(Location location, const std::string& message, MessageType message_type) {
    if (!Settings::ostream)
        return;
    if (!Settings::current_loglevel->has_message_type(message_type))
        return;

    *Settings::ostream
        << location.to_string() << line_col_padding(location.to_string().size()) 
        << "info:  " << message << '\n';
}

} //namespace logging