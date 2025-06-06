#ifndef __WORDS_HH
#define __WORDS_HH

/**
 * This file defines some properties of the language, primarily to be used by the lexer
 */

#include <string>
#include <array>
#include <algorithm>

constexpr std::string_view OPERATOR_GLYPHS = "+-*/%^=!?|<>.$&€£@¬§¤";
constexpr std::string_view GLYPHS_FORBIDDEN_IN_NAMES = "+-*/%^=|<>.$&€£@¬§¤;:\\#~[]{}()\"` \n";

// NOTE: out of date, actually implemented keywords can be found in mapsc/parser/token.cpp
constexpr std::array RESERVED_WORDS = {
    "return",
    "let", "mut", "const", "type", "class",
    "if", "else",
    "for", "while", "do",
    "match", "case", "switch", "default",
    "from",
    "has", "in", "of",                          // has could be a builtin
    "with",
    "is", "typeof", "derives", "from",
    "extern",
    "async", "await", "maybe", "value", "fail", // might be builtins
    "operator", "unary", "binary", "prefix", "infix", "postfix"
};

constexpr inline bool is_operator_glyph(char glyph) {
    return OPERATOR_GLYPHS.find(glyph) != std::string::npos;
}

constexpr inline bool is_reserved_word(const std::string& word) {
    return std::find(RESERVED_WORDS.begin(), RESERVED_WORDS.end(), word) != RESERVED_WORDS.end();
}

constexpr inline bool is_allowed_in_identifiers(char ch) {
    if (isalnum(ch))
        return true;

    if (std::find(GLYPHS_FORBIDDEN_IN_NAMES.begin(), GLYPHS_FORBIDDEN_IN_NAMES.end(), ch) != 
        GLYPHS_FORBIDDEN_IN_NAMES.end())
            return false;

    return true;
}

#endif