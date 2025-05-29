#include "implementation.hh"

#include <optional>

#include "mapsc/ast/misc_expression.hh"
#include "mapsc/ast/statement.hh"

using std::optional, std::nullopt, std::to_string;

namespace Maps {

using Log = LogInContext<LogContext::layer1>;

Statement* ParserLayer1::parse_if_statement() {
    auto location = current_token().location;
    auto initial_if_indent = indent_level_;

    Log::debug_extra("Parsing if statement", current_token().location);

    get_token(); // eat the if

    auto condition = parse_condition_expression();
    if (!result_.success)
        return fail_statement("Parsing if statement condition failed", condition->location);

    Log::debug_extra("Parsed if statement condition: " + condition->log_message_string(), 
        current_token().location);

    auto body = parse_conditional_body();
    if (!result_.success)
        return fail_statement("Parsing if statement body failed", body->location);

    Log::debug_extra("Parsed if statement body: " + body->log_message_string(), 
        current_token().location);

    if (current_token().token_type != TokenType::else_t) {
        Log::debug_extra("Finished parsing if statement from " + location.to_string(), 
            current_token().location);
        return create_if(*ast_store_, condition, body, location);
    }

    // handle else(s)
    auto else_branch = parse_else_branch(initial_if_indent);
    if (has_failed())
        return fail_statement("Parsing if statement else branch failed", body->location);

    Log::debug_extra("Finished parsing if statement from " + location.to_string(), 
        current_token().location);

    return create_if_else(*ast_store_, condition, body, else_branch, location);
}

Statement* ParserLayer1::parse_else_branch(uint initial_indent) {
    get_token(); // eat the else

    Log::debug_extra("Parsing else statement", current_token().location);

    bool indented = indent_level_ > initial_indent;

    auto else_branch = parse_statement();
    if (has_failed())
        return else_branch;

    if (indented)
        while (indent_level_ > initial_indent && current_token().token_type == TokenType::indent_block_end && !eof())
            get_token();

    if (indent_level_ < initial_indent && !eof()) {
        auto location = current_token().location;
        Log::debug_extra("Mismatched indents: " + to_string(indent_level_) + 
            " and " + to_string(initial_indent), 
            location);

        return fail_statement("Mismatched indents at the end of else statement", 
            location);
    }

    return else_branch;
}

Expression* ParserLayer1::parse_condition_expression() {
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

Statement* ParserLayer1::parse_conditional_body() {
    switch (current_token().token_type) {
        case TokenType::then:
        case TokenType::do_t:
        case TokenType::semicolon:
            get_token();
            return parse_conditional_body();

        case TokenType::indent_block_start: {
            if (!is_condition_ender(peek()))
                return parse_statement();
            
            // handle block starting with then
            get_token(); // eat indent start
            get_token(); // eat "then"

            auto body = parse_statement();
            if (!result_.success)
                return body;
            
            switch (current_token().token_type) {
                case TokenType::indent_block_end:
                    get_token(); // eat indent end
                    break;
                case TokenType::eof:
                case TokenType::else_t:
                    break;
                default:
                    return fail_statement("Mismatched indents in if statement body", 
                        current_token().location);
            }

            return body;
        }
        default:
            return parse_statement();
    }
}

Statement* ParserLayer1::parse_guard_statement() {
    auto location = current_token().location;
    get_token();

    auto condition = parse_expression();
    if (has_failed())
        return fail_statement("Parsing guard statement failed", condition->location);

    return create_guard(*ast_store_, condition, location);
}

Statement* ParserLayer1::parse_switch_statement() {
    get_token();

    assert(false && "not implemented");
    // return create_
}

} // namespace Maps