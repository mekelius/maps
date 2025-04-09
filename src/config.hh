#ifndef __CONFIG_HH
#define __CONFIG_HH

#include <string>
#include <array>

// at line 1000 the token stream is gonna shift right, but that's ok
constexpr unsigned int LINE_COL_FORMAT_PADDING = 8;

inline std::string line_col_padding(unsigned int width) {
    return ( width < LINE_COL_FORMAT_PADDING ? 
            std::string(LINE_COL_FORMAT_PADDING - width, ' ') : " ");
}

constexpr bool REVERSE_PARSE_INCLUDE_DEBUG_INFO = true;
constexpr unsigned int REVERSE_PARSE_INDENT_WIDTH = 4;

namespace Logging {

enum class MessageType: unsigned int {
    error = 0,
    general_info,
    parser_debug,
};

struct LogLevel {
    std::array<bool, 5> message_types = {
        true,
        true,
        false,
    };

    void set_message_type(MessageType message_type, bool value) {
        message_types.at(static_cast<unsigned int>(message_type)) = value;
    }

    static void set(LogLevel& level) {
        current = &level;
    }
    static bool has_message_type(MessageType message_type) {
        return current->message_types[static_cast<unsigned int>(message_type)];
    }
    
    static LogLevel* current;

    static LogLevel quiet;
    static LogLevel default_;
    static LogLevel everything;
};

void init(LogLevel& log_level = LogLevel::default_);

} // namespace Logging


#endif