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

    Log::debug_extra(current_token().location) << "Parsing if statement" << Endl;

    get_token(); // eat the if

    auto condition = parse_condition_expression();
    if (!result_.success) {
            Log::error(condition->location) << "Parsing if statement condition failed" << Endl;
            return fail_statement(condition->location);
    }
    Log::debug_extra(current_token().location) << "Parsed if statement condition: " << *condition << Endl;

    auto body = parse_conditional_body();
    if (!result_.success) {
        Log::error(body->location) << "Parsing if statement body failed" << Endl;
        return fail_statement(body->location);
    }

    Log::debug_extra(current_token().location) << "Parsed if statement body: " << *body << Endl;

    if (current_token().token_type != TokenType::else_t) {
        Log::debug_extra(current_token().location) << "Finished parsing if statement from " << location << Endl;
        return create_if(*ast_store_, condition, body, location);
    }

    // handle else(s)
    auto else_branch = parse_else_branch(initial_if_indent);
    if (has_failed()) {
        Log::error(body->location) << "Parsing if statement else branch failed" << Endl;
        return fail_statement(body->location);
    }

    Log::debug_extra(current_token().location) << "Finished parsing if statement from " << location << Endl;

    return create_if_else(*ast_store_, condition, body, else_branch, location);
}

Statement* ParserLayer1::parse_else_branch(uint initial_indent) {
    get_token(); // eat the else

    Log::debug_extra(current_token().location) << "Parsing else statement" << Endl;

    bool indented = indent_level_ > initial_indent;

    auto else_branch = parse_statement();
    if (has_failed())
        return else_branch;

    if (indented)
        while (indent_level_ > initial_indent && current_token().token_type == TokenType::indent_block_end && !eof())
            get_token();

    if (indent_level_ < initial_indent && !eof()) {
        auto location = current_token().location;
        Log::error(location) <<
            "Mismatched indents: " << indent_level_ << " and " << initial_indent;

        return fail_statement(location);
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
                    Log::error(current_token().location) << "Mismatched indents in if statement body" << Endl;
                    return fail_statement(current_token().location);
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
    if (has_failed()) {
        Log::error(condition->location) << "Parsing guard statement failed" << Endl;
        return fail_statement(condition->location);
    }

    return create_guard(*ast_store_, condition, location);
}

Statement* ParserLayer1::parse_switch_statement() {
    get_token();

    assert(false && "not implemented");
    // return create_
}

} // namespace Maps