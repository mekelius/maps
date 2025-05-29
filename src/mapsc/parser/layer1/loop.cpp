#include "implementation.hh"

#include <optional>

#include "mapsc/ast/misc_expression.hh"
#include "mapsc/ast/statement.hh"

using std::optional, std::nullopt, std::to_string;

namespace Maps {

using Log = LogInContext<LogContext::layer1>;

Statement* ParserLayer1::parse_while_loop() {
    auto location = current_token().location;

    Log::debug_extra("Parsing while loop", location);

    get_token();

    auto condition = parse_condition_expression();
    if (has_failed())
        return fail_statement("Parsing while loop failed", condition->location);
    
    Log::debug_extra("Parsing while loop body", current_token().location);

    auto body = parse_conditional_body();
    if (has_failed())
        return fail_statement("Parsing while loop failed", body->location);

    return create_while(*ast_store_, condition, body, location);
}

Statement* ParserLayer1::parse_for_loop() {
    get_token();

    assert(false && "not implemented");
    // return create_
}

Statement* ParserLayer1::parse_yield_statement() {
    get_token();

    assert(false && "not implemented");
    // return create_
}

} // namespace Maps