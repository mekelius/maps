#include "implementation.hh"

#include <cassert>
#include <sstream>

#include "mapsc/logging.hh"

#include "mapsc/ast/identifier.hh"
#include "mapsc/ast/expression.hh"
#include "mapsc/ast/value.hh"

#include "mapsc/parser/token.hh"

using std::optional, std::nullopt, std::make_unique;


namespace Maps {

using Log = LogInContext<LogContext::layer1>;

Expression* ParserLayer1::handle_string_literal() {
    Expression* expression = create_string_literal(*ast_store_, current_token().string_value(), 
        current_token().location);

    get_token();
    
    Log::debug_extra(expression->location) << "Parsed string literal";
    return expression;
}

Expression* ParserLayer1::handle_numeric_literal() {
    Expression* expression = create_numeric_literal(*ast_store_, current_token().string_value(), 
        current_token().location);
    
    get_token();
        
    Log::debug_extra(expression->location) << "Parsed numeric literal";
    return expression;
}

Expression* ParserLayer1::handle_identifier() {
    Expression* expression = Maps::create_identifier(*ast_store_, parse_scope_, 
        current_token().string_value(), current_token().location);
    result_.unresolved_identifiers.push_back(expression);

    get_token();
    
    Log::debug_extra(expression->location) << "Parsed unresolved identifier";
    return expression;
}

Expression* ParserLayer1::handle_type_identifier() {
    Expression* expression = create_type_identifier(*ast_store_, 
        current_token().string_value(), current_token().location);
    result_.unresolved_type_identifiers.push_back(expression);

    get_token();
    
    Log::debug_extra(expression->location) << "Parsed unresolved type identifier";
    return expression;
}

Expression* ParserLayer1::handle_type_constructor_identifier() {
    assert(false && "not implemented");
}


} // namespace Maps
