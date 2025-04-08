#ifndef __CONFIG_HH
#define __CONFIG_HH

#include <string>

// at line 1000 the token stream is gonna shift right, but that's ok
constexpr unsigned int LINE_COL_FORMAT_PADDING = 8;

constexpr bool REVERSE_PARSE_INCLUDE_DEBUG_INFO = true;
constexpr bool VERIFY_OUTPUT_TOKENS = false;
constexpr unsigned int REVERSE_PARSE_INDENT_WIDTH = 4;

inline std::string line_col_padding(unsigned int width) {
    return ( width < LINE_COL_FORMAT_PADDING ? 
            std::string(LINE_COL_FORMAT_PADDING - width, ' ') : " ");
}

#endif