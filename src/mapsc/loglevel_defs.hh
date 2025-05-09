#ifndef __LOGLEVEL_DEFS_HH
#define __LOGLEVEL_DEFS_HH

#include <array>

namespace Maps {

// clang-format off
constexpr auto MESSAGE_TYPES_START_LINE = __LINE__;
enum class MessageType {
    error = 0,
    general_info = 1,
    parser_debug = 2,
    parser_debug_terminal = 3,
    parser_debug_identifier = 4,
    lexer_debug_token = 5,
    pragma_debug = 6,
    parser_debug_termed_expression = 7,
    ir_gen_debug = 8,
    post_parse_debug = 9
};
constexpr auto MESSAGE_TYPE_COUNT = __LINE__ - MESSAGE_TYPES_START_LINE - 3;
// clang-format on
using MessageTypes = std::array<bool, 10>;
static_assert(sizeof(MessageTypes) == MESSAGE_TYPE_COUNT);

struct LogLevel {
    constexpr static MessageTypes nothing() {
        MessageTypes message_types;
        std::fill(message_types.begin(), message_types.end(), false);
        return message_types;
    }

    constexpr static MessageTypes quiet() {
        MessageTypes message_types;
        std::fill(message_types.begin(), message_types.end(), false);
        message_types.at(static_cast<int>(MessageType::error)) = true;
        return message_types;
    }

    constexpr static MessageTypes debug() {
        return {
            true,  // error = 0
            true,  // general_info = 1
            true,  // parser_debug = 2
            false, // parser_debug_terminal = 3
            false, // parser_debug_identifier = 4
            false, // lexer_debug_token = 5
            true,  // pragma_debug = 6
            true,  // parser_debug_termed_expression = 7
            true,  // ir_gen_debug = 8
            true,  // post_parse_debug = 9
        };
    }

    constexpr static MessageTypes everything() {
        MessageTypes message_types;
        std::fill(message_types.begin(), message_types.end(), true);
        return message_types;
    }

    constexpr static MessageTypes default_() {
        return {
            true,  // error = 0
            true,  // general_info = 1
            false, // parser_debug = 2
            false, // parser_debug_terminal = 3
            false, // parser_debug_identifier = 4
            false, // lexer_debug_token = 5
            false, // pragma_debug = 6
            false, // parser_debug_termed_expression = 7
            false, // ir_gen_debug = 8
            false, // post_parse_debug = 9
        };
    }
    
    LogLevel() = delete;
};

} // namespace Logging

#endif