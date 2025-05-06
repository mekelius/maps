#ifndef __REVERSE_PARSE_HH
#define __REVERSE_PARSE_HH

#include <ostream>

#include "mapsc/ast/ast_store.hh"

class ReverseParser final {
public:
    struct Options {
        bool include_debug_info = false;
        bool debug_separators = false;
        unsigned int indent_width = 4;
    };

    ReverseParser(std::ostream* ostream)
        :ostream_(ostream) {}
    ReverseParser(std::ostream* ostream, const Options& options)
        :ostream_(ostream), options_(options) {}

    void set_options(const Options& options) { options_ = options; }

    ReverseParser& operator<<(const std::string& str) { *ostream_ << str; return *this; }
    ReverseParser& operator<<(const char ch) { *ostream_ << ch; return *this; }
    ReverseParser& operator<<(Maps::AST_Store& ast) { return reverse_parse(ast); }
    ReverseParser& operator<<(Maps::CallableBody body) { return print_callable(body); }
    ReverseParser& operator<<(Maps::Expression& expression) { return print_expression(expression); }
    ReverseParser& operator<<(Maps::Statement& statement) { return print_statement(statement); }
private:
    void reset();
    std::string linebreak();

    ReverseParser& reverse_parse(Maps::AST_Store& ast);
    ReverseParser& print_statement(const Maps::Statement& statement);
    ReverseParser& print_expression(Maps::Expression& expression);
    ReverseParser& print_callable(Maps::CallableBody body);

    std::ostream* ostream_;

    Options options_ = {};
    
    bool skipped_initial_linebreak_doubling_ = false;
    unsigned int indent_stack_ = 0;
};

#endif