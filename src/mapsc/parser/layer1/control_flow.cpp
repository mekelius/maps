#include "implementation.hh"

#include <optional>

#include "mapsc/ast/misc_expression.hh"
#include "mapsc/ast/statement.hh"

using std::optional, std::nullopt;

namespace Maps {

using Log = LogInContext<LogContext::layer1>;

Statement* ParserLayer1::parse_if_statement() {
    auto location = current_token().location;

    Log::debug_extra("Parsing if statement", current_token().location);

    auto initial_branch = parse_if_branch();
    if (!result_.success)
        return initial_branch.second;

    IfChain chain{ initial_branch };
    optional<Statement*> final_else = nullopt;

    // handle else(s)
    while (current_token().token_type == TokenType::else_t) {
        auto location = current_token().location;
        get_token(); // eat the else

        if (current_token().token_type == TokenType::if_t) {
            Log::debug_extra("Parsing else if statement", location);
            auto branch = parse_if_branch();
            if (!result_.success)
                return branch.second;

            chain.push_back(branch);
            continue;
        }

        Log::debug_extra("Parsing else statement", location);

        final_else = parse_statement();
        if (!result_.success)
            return *final_else;

        Log::debug_extra("Parsed else statement: " + (*final_else)->log_message_string(), location);
        break;
    }

    return create_if_else_chain(*ast_store_, chain, final_else, location);
}

IfBranch ParserLayer1::parse_if_branch() {
    auto location = current_token().location;
    get_token();

    auto condition = parse_if_statement_condition();
    if (!result_.success) {
        auto location = condition->location;
        fail("Parsing if statement condition failed", condition->location);
        return {create_user_error(*ast_store_, location), 
            create_user_error_statement(*ast_store_, location)};
    }

    Log::debug_extra("Parsed if statement condition: " + condition->log_message_string(), 
        current_token().location);

    auto body = parse_if_statement_body();
    if (!result_.success) {
        fail("Parsing if statement body failed", body->location);
        return {create_user_error(*ast_store_, location), 
            create_user_error_statement(*ast_store_, location)};
    }

    Log::debug_extra("Parsed if statement body: " + body->log_message_string(), 
        current_token().location);

    return {condition, body};
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