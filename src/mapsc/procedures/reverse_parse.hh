#ifndef __REVERSE_PARSE_HH
#define __REVERSE_PARSE_HH

#include <ostream>

#include "mapsc/ast/ast_store.hh"

namespace Maps {

class ReverseParser final {
public:
    struct Options {
        bool include_debug_info = false;
        bool debug_separators = false;
        unsigned int indent_width = 4;
        bool include_all_types = false;
    };

    ReverseParser(std::ostream* ostream);
    ReverseParser(std::ostream* ostream, const Options& options);

    void set_options(const Options& options) { options_ = options; }

    ReverseParser& operator<<(const std::string& str) { *ostream_ << str; return *this; }
    ReverseParser& operator<<(const char ch) { *ostream_ << ch; return *this; }
    ReverseParser& operator<<(const CompilationState& state) { return reverse_parse(state); }
    ReverseParser& operator<<(CallableBody body) { return print_callable(body); }
    ReverseParser& operator<<(Expression& expression) { return print_expression(expression); }
    ReverseParser& operator<<(Statement& statement) { return print_statement(statement); }
    ReverseParser& operator<<(maps_Int val) { *ostream_ << val; return *this; }
    ReverseParser& operator<<(maps_Float val) { *ostream_ << val; return *this; }

private:
    void reset();
    std::string linebreak();

    ReverseParser& reverse_parse(const CompilationState& state);
    ReverseParser& print_statement(const Statement& statement);
    ReverseParser& print_expression(Expression& expression);
    ReverseParser& print_callable(CallableBody body);

    ReverseParser& print_type_declaration(Expression& expression);

    std::ostream* ostream_;

    Options options_ = {};
    
    bool skipped_initial_linebreak_doubling_ = false;
    unsigned int indent_stack_ = 0;
};

} // namespace Maps

#endif