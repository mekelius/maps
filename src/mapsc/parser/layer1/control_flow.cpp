#include "implementation.hh"

#include "mapsc/ast/statement.hh"

namespace Maps {

using Log = LogInContext<LogContext::layer1>;

Statement* ParserLayer1::parse_if_statement() {
    auto location = current_token().location;
    get_token();

    auto condition = parse_if_statement_condition();
    if (!result_.success)
        return fail_statement("Parsing if statement condition failed", condition->location);

    Log::debug_extra("Parsed if statement condition: " + condition->log_message_string(), 
        current_token().location);

    auto branch = parse_if_statement_body();
    if (!result_.success)
        return fail_statement("Parsing if statement body failed", branch->location);

    Log::debug_extra("Parsed if statement body: " + branch->log_message_string(), 
        current_token().location);

    return create_if(*ast_store_, condition, branch, location);
}

Expression* ParserLayer1::parse_if_statement_condition() {
    switch (current_token().token_type) {
        case TokenType::parenthesis_open: {
            get_token(); // eat the '('
            auto condition = parse_expression();
            get_token(); // eat the ')'
            return condition;
        }

        default:
            return parse_expression();
    }
}

Statement* ParserLayer1::parse_if_statement_body() {
    switch (current_token().token_type) {
        case TokenType::then:
        case TokenType::semicolon:
            get_token();
            return parse_if_statement_body();

        case TokenType::indent_block_start: {
            if (peek().token_type != TokenType::then)
                return parse_statement();
            
            // handle block starting with then
            get_token(); // eat indent start
            get_token(); // eat "then"

            auto body = parse_statement();
            if (!result_.success)
                return body;
            
            if (current_token().token_type != TokenType::indent_block_end)
                return fail_statement("Mismatched indents in if statement body", 
                    current_token().location);

            get_token(); // eat indent end
            return body;
        }
        default:
            return parse_statement();
    }
}

Statement* ParserLayer1::parse_while_loop() {
    get_token();

    assert(false && "not implemented");
    // return create_
}
Statement* ParserLayer1::parse_for_loop() {
    get_token();

    assert(false && "not implemented");
    // return create_
}
Statement* ParserLayer1::parse_guard_statement() {
    get_token();

    assert(false && "not implemented");
    // return create_
}
Statement* ParserLayer1::parse_switch_statement() {
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