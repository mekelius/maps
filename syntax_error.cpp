#include "syntax_error.hh"

std::string syntax_error(unsigned int line_number, const std::string& message) {
    return "syntax error on line " + std::to_string(line_number) + ": " + message;
}