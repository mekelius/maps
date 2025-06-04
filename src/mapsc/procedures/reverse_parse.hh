#ifndef __REVERSE_PARSE_HH
#define __REVERSE_PARSE_HH

#include <ostream>
#include <string>

#include "common/maps_datatypes.h"

#include "mapsc/ast/definition.hh"
#include "mapsc/ast/definition_body.hh"
#include "mapsc/ast/function_definition.hh"

namespace Maps {

class ReverseParser final {
public:
    struct Options {
        bool include_debug_info = false;
        bool debug_separators = false;
        bool debug_node_types = false;
        unsigned int indent_width = 4;
        bool include_all_types = false;
    };

    ReverseParser(std::ostream* ostream);
    ReverseParser(std::ostream* ostream, const Options& options);

    ReverseParser(LogStream* logstream);
    ReverseParser(LogStream* logstream, const Options& options);

    void set_options(const Options& options) { options_ = options; }

    ReverseParser& operator<<(std::string_view str) { *ostream_ << str; return *this; }
    ReverseParser& operator<<(const char ch) { *ostream_ << ch; return *this; }
    ReverseParser& operator<<(const Scope& scope) { return reverse_parse(scope); }
    ReverseParser& operator<<(const DefinitionHeader& definition) { return print_definition(definition); }
    ReverseParser& operator<<(const DefinitionBody& body) { return print_definition(body); }
    ReverseParser& operator<<(const LetDefinitionValue& body) { return print_definition(body); }
    ReverseParser& operator<<(const Expression& expression) { return print_expression(expression); }
    ReverseParser& operator<<(const Statement& statement) { return print_statement(statement); }
    ReverseParser& operator<<(const ParameterList& parameters) { return print_parameter_list(parameters); }
    ReverseParser& operator<<(maps_Int val) { *ostream_ << val; return *this; }
    ReverseParser& operator<<(maps_Float val) { *ostream_ << val; return *this; }

private:
    void reset();
    std::string linebreak();

    ReverseParser& reverse_parse(const Scope& scope);
    ReverseParser& print_definition(const DefinitionHeader& definition);
    ReverseParser& print_definition(const DefinitionBody& definition);
    ReverseParser& print_definition(const LetDefinitionValue& definition);
    ReverseParser& print_statement(const Statement& statement);
    ReverseParser& print_expression(const Expression& expression);
    ReverseParser& print_parameter_list(const ParameterList& parameters);

    ReverseParser& print_type_declaration(const Expression& expression);

    std::ostream* ostream_;

    Options options_ = {};
    
    bool skipped_initial_linebreak_doubling_ = false;
    unsigned int indent_stack_ = 0;
    bool prevent_next_linebreak_ = false;
};

} // namespace Maps

#endif