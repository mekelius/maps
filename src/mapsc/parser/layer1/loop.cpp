#include "implementation.hh"

#include <optional>

#include "mapsc/ast/misc_expression.hh"
#include "mapsc/ast/statement.hh"

using std::optional, std::nullopt, std::to_string;

namespace Maps {

using Log = LogInContext<LogContext::layer1>;

Statement* ParserLayer1::parse_while_loop() {
    auto location = current_token().location;
    auto initial_indent = indent_level_;

    Log::debug_extra(location) << "Parsing while loop";

    get_token();

    auto condition = parse_condition_expression();
    if (has_failed()) {
        Log::error(condition->location) << "Parsing while loop failed";
        return fail_statement(condition->location);
    }
    
    Log::debug_extra(current_token().location) << "Parsing while loop body";

    auto body = parse_conditional_body();
    if (has_failed()) {
        Log::error(body->location) << "Parsing while loop failed";
        return fail_statement(body->location);
    }

    if (current_token().token_type != TokenType::else_t) {
        Log::debug_extra(current_token().location) << "Finished parsing while loop from " << location;
        return create_while(*ast_store_, condition, body, location);
    }

    Log::debug_extra(current_token().location) << "Parsing else branch for a while loop";

    auto else_branch = parse_else_branch(initial_indent);
    if (has_failed()) {
        Log::error(body->location) << "Parsing while loop else branch failed";
        return fail_statement(body->location);
    }

    Log::debug_extra(current_token().location) << "Finished parsing while loop from " << location;

    return create_while_else(*ast_store_, condition, body, else_branch, location);
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