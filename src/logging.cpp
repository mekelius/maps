#include "logging.hh"

namespace Logging {

LogLevel LogLevel::nothing;
LogLevel LogLevel::quiet;
LogLevel LogLevel::default_;
LogLevel LogLevel::debug;
LogLevel LogLevel::everything;

LogLevel Settings::current_loglevel = LogLevel::default_;

std::ostream* Settings::ostream;
std::ofstream* Settings::tokens_ofstream;

bool log_check_flag = false;

void LogLevel::init(LogLevel log_level) {    
    for (auto it = LogLevel::nothing.message_types.begin(); it != LogLevel::nothing.message_types.end(); it++) {
        *it = false;
    }
    
    LogLevel::quiet.set_message_type(MessageType::general_info, false);
    
    for (auto it = LogLevel::everything.message_types.begin(); it != LogLevel::everything.message_types.end(); it++) {
        *it = true;
    }
    
    LogLevel::debug.set_message_type(MessageType::parser_debug, true);
    LogLevel::debug.set_message_type(MessageType::parser_debug_identifier, true);
    LogLevel::debug.set_message_type(MessageType::pragma_debug, true);

    Settings::set_loglevel(log_level);
}

void log_error(SourceLocation location, const std::string& message) {
    if (!Settings::ostream)
        return;
    if (!Settings::current_loglevel.has_message_type(MessageType::error))
        return;

    log_check_flag = true;

    *Settings::ostream
        << location.to_string() << line_col_padding(location.to_string().size()) 
        << "error: " << message << "\n";
}

void log_info(SourceLocation location, const std::string& message, MessageType message_type) {
    if (!Settings::ostream)
        return;
    if (!Settings::current_loglevel.has_message_type(message_type))
        return;

    log_check_flag = true;

    *Settings::ostream
        << location.to_string() << line_col_padding(location.to_string().size()) 
        << "info:  " << message << '\n';
}

void log_token(SourceLocation location, const std::string& message) {
    if (Settings::ostream && Settings::current_loglevel.has_message_type(MessageType::lexer_debug_token)) {
        log_check_flag = true;

        *Settings::ostream
            << location.to_string() << line_col_padding(location.to_string().size()) 
            << "token: " << message << '\n';
    }

    if (Settings::tokens_ofstream) {
        log_check_flag = true;

        *Settings::ostream
            << location.to_string() << line_col_padding(location.to_string().size()) 
            << "token: " << message << '\n';
    }
}

bool logs_since_last_check() {
    bool value = log_check_flag;
    log_check_flag = false;
    return value;
}

} //namespace logging