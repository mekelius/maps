#ifndef __LOGGING_HH
#define __LOGGING_HH

#include <string>
#include <ostream>
#include <array>

#include "mapsc/source.hh"

namespace Logging {

// at line 1000 the token stream is gonna shift right, but that's ok
constexpr unsigned int LINE_COL_FORMAT_PADDING = 8;

inline std::string line_col_padding(unsigned int width) {
    return ( width < LINE_COL_FORMAT_PADDING ? 
            std::string(LINE_COL_FORMAT_PADDING - width, ' ') : " ");
}

enum class MessageType: unsigned int {
    error = 0,
    general_info = 1,
    parser_debug = 2,
    parser_debug_terminal = 3,
    parser_debug_identifier = 4,
    lexer_debug_token = 5,
    pragma_debug = 6,
    parser_debug_termed_expression = 7,
    ir_gen_debug = 8,
};

class LogLevel {
  public:
    static LogLevel nothing;
    static LogLevel quiet;
    static LogLevel default_;
    static LogLevel debug;
    static LogLevel everything;
  
    static void init(LogLevel loglevel = LogLevel::default_);

    bool has_message_type(MessageType message_type) {
        return message_types[static_cast<unsigned int>(message_type)];
    }

    void set_message_type(MessageType message_type, bool value) {
        message_types.at(static_cast<unsigned int>(message_type)) = value;
    }

    std::array<bool, 9> message_types = {
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        false,
        false,
    };
};

struct Settings {
    static void set_loglevel(LogLevel level) {
        Settings::current_loglevel = level;
    }

    static void set_ostream(std::ostream* ostream) {
        Settings::ostream = ostream;
    }

    static void set_tokens_ofstream(std::ofstream* ofstream) {
        Settings::tokens_ofstream = ofstream;
    }

    static void set_message_type(MessageType message_type, bool value) {
        Settings::current_loglevel.set_message_type(message_type, value);
    }

    static LogLevel current_loglevel;
    static std::ostream* ostream; // NOTE: allowed to be nullptr
    static std::ofstream* tokens_ofstream; // NOTE: allowed to be nullptr
};

// pass nullptr to disable logging
// TODO: incorporate lexer into this
inline void init_logging(std::ostream* ostream, LogLevel& log_level = LogLevel::default_) {
    LogLevel::init(log_level);
    Settings::set_ostream(ostream);
}

void log_error(const std::string& message, SourceLocation location = NO_SOURCE_LOCATION);

void log_info(const std::string& message, MessageType message_type, SourceLocation location = NO_SOURCE_LOCATION);
void log_token(const std::string& message, SourceLocation location);

// returns true if log output has happened since last time this was called
// only call this from main as a formatting aid
bool logs_since_last_check();

} // namespace Logging

#endif