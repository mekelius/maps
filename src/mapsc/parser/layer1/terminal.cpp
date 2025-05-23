/**
 * Recursive descent parser for maps programming language
 * 
 * The convention here (unlike in the lexer) is that every production rule must move the buffer
 * beyond the tokens it consumed.
 * 
 * Same if a token is rejected. 
 */
#include "../parser_layer1.hh"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <sstream>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/pragma.hh"
#include "mapsc/compilation_state.hh"

#include "mapsc/types/type.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/ast/operator.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/ast_store.hh"

#include "mapsc/parser/token.hh"
#include "mapsc/procedures/simplify.hh"

using std::optional, std::nullopt, std::make_unique;


namespace Maps {

using Log = LogInContext<LogContext::layer1>;

Expression* ParserLayer1::handle_string_literal() {
    Expression* expression = Expression::string_literal(*ast_store_, current_token().string_value(), 
        current_token().location);

    get_token();
    
    log("parsed string literal", LogLevel::debug_extra);
    return expression;
}

Expression* ParserLayer1::handle_numeric_literal() {
    Expression* expression = Expression::numeric_literal(*ast_store_, current_token().string_value(), 
        current_token().location);
    
    get_token();
        
    log("parsed numeric literal", LogLevel::debug_extra);
    return expression;
}

Expression* ParserLayer1::handle_identifier() {
    Expression* expression = Expression::identifier(*ast_store_, parse_scope_, 
        current_token().string_value(), current_token().location);
    result_.unresolved_identifiers.push_back(expression);

    get_token();
    
    log("parsed unresolved identifier", LogLevel::debug_extra);
    return expression;
}

Expression* ParserLayer1::handle_type_identifier() {
    Expression* expression = Expression::type_identifier(*ast_store_, 
        current_token().string_value(), current_token().location);
    result_.unresolved_type_identifiers.push_back(expression);

    get_token();
    
    log("parsed unresolved type identifier", LogLevel::debug_extra);
    return expression;
}

Expression* ParserLayer1::handle_type_constructor_identifier() {
    assert(false && "not implemented");
}


} // namespace Maps
