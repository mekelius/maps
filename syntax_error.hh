#ifndef __SYNTAX_ERROR_HH
#define __SYNTAX_ERROR_HH

#include <string>

// formatting function for syntax errors
std::string syntax_error(unsigned int line_nbr, const std::string& message);

#endif