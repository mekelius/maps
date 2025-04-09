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
constexpr bool VERIFY_OUTPUT_TOKENS = true;
constexpr unsigned int REVERSE_PARSE_INDENT_WIDTH = 4;

namespace LogLevel {

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
        false,
        false
    };
};

constexpr LogLevel quiet = {{
    false,
    false,
    false,
    false,
    false,
}};
constexpr LogLevel default_;
constexpr LogLevel everything = {{
    true,
    true,
    true,
    true,
    true,
}};

constexpr LogLevel current = everything;

constexpr inline bool has_message_type(MessageType message_type) {
    return current.message_types[static_cast<unsigned int>(message_type)];
}

} // namespace LogLevel


#endif