#ifndef __LOGGING_HH
#define __LOGGING_HH

#include <string>
#include <ostream>
#include <array>

constexpr bool REVERSE_PARSE_INCLUDE_DEBUG_INFO = true;
constexpr unsigned int REVERSE_PARSE_INDENT_WIDTH = 4;

struct SourceLocation {
    unsigned int line;
    unsigned int column;

    std::string to_string() const {
        return std::to_string(line) + ":" + std::to_string(column);
    };
};

namespace Logging {

// at line 1000 the token stream is gonna shift right, but that's ok
constexpr unsigned int LINE_COL_FORMAT_PADDING = 8;

inline std::string line_col_padding(unsigned int width) {
    return ( width < LINE_COL_FORMAT_PADDING ? 
            std::string(LINE_COL_FORMAT_PADDING - width, ' ') : " ");
}

enum class MessageType: unsigned int {
    error = 0,
    general_info,
    parser_debug,
    parser_debug_terminal,
    lexer_debug_token,
};

class LogLevel {
  public:
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

  private:
    std::array<bool, 5> message_types = {
        true,
        true,
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

void log_error(SourceLocation location, const std::string& message);
void log_info(SourceLocation location, const std::string& message, MessageType message_type);
void log_token(SourceLocation location, const std::string& message);

} // namespace Logging


#endif