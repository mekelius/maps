#ifndef __WORDS_HH
#define __WORDS_HH

/**
 * This file defines some properties of the language, primarily to be used by the lexer
 */

#include <string>
#include <array>
#include <algorithm>

// TODO: make these constexpr
constexpr std::string_view OPERATOR_GLYPHS = "+-*/^=!?|<>.$&€£@¬§¤";

// OPT: roll these into a single enum
constexpr std::array<std::string_view, 42> RESERVED_WORDS = {
    "return",
    "let", "mut", "const", "type", "class",
    "if", "else",
    "for", "while", "do",
    "match", "case", "switch", "default",
    "from",
    "has", "in", "of",                          // has could be a builtin
    "with",
    "not", "and", "or", "xor", "nor", "nand",   // these could be builtins
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

#endif