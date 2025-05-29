#include "implementation.hh"

#include <cassert>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/parser/token.hh"

using std::optional, std::nullopt, std::make_unique;


namespace Maps {

using Log = LogInContext<LogContext::layer1>;

Statement* ParserLayer1::parse_statement() {
    auto location = current_token().location;

    switch (current_token().token_type) {
        case TokenType::eof:
            return create_empty_statement(*ast_store_, location);

        case TokenType::identifier:
            if (peek().token_type == TokenType::operator_t && peek().string_value() == "=") {
                return parse_assignment_statement();
            }
            return parse_expression_statement();

        case TokenType::indent_block_start:
        case TokenType::curly_brace_open:
            return parse_block_statement();

        case TokenType::type_identifier:
        case TokenType::lambda:
        case TokenType::question_mark:
        case TokenType::parenthesis_open:
        case TokenType::bracket_open:
        case TokenType::operator_t:
        case TokenType::string_literal:
        case TokenType::number:
            return parse_expression_statement();

        case TokenType::semicolon:
            get_token(); // eat the semicolon
            return create_empty_statement(*ast_store_, location);

        // ----- ignored -----
        case TokenType::dummy:
            get_token(); // eat the dummy
            return create_empty_statement(*ast_store_, location);

        case TokenType::indent_block_end:
            assert(false && "parse_statement called at indent_block_end");
            return fail_statement("Parse_statement called at indent_block_end", location, true);
        
        case TokenType::indent_error_fatal:
            return fail_statement("Indent error", location);

        case TokenType::return_t:
            return parse_return_statement();

        case TokenType::let:
            return parse_inner_let_definition();

        case TokenType::if_t:
            return parse_if_statement();
        case TokenType::while_t:
            return parse_while_loop();
        case TokenType::for_t:
            return parse_for_loop();
        case TokenType::guard:
            return parse_guard_statement();
        case TokenType::switch_t:
            return parse_switch_statement();
        case TokenType::yield_t:
            return parse_yield_statement();

        // ---- errors -----
        default:
            return fail_statement(
                "Unexpected "+ current_token().get_string() + ", expected a statement", location);
    }
}

Statement* ParserLayer1::parse_expression_statement() {
    auto location = current_token().location;

    Expression* expression = parse_expression();
    Statement* statement = create_expression_statement(*ast_store_, expression, location);

    if(!is_block_starter(current_token()) && !is_statement_separator(current_token()))
        return fail_statement("Statement didn't end in a statement separator", 
            current_token().location, true);

    if (current_token().token_type == TokenType::semicolon)
        get_token(); // eat trailing semicolon
    log("Finished parsing expression statement from " + statement->location.to_string(), 
        LogLevel::debug_extra);
    return statement;
}

Statement* ParserLayer1::parse_assignment_statement() {
    auto location = current_token().location;
    bool is_top_level = in_top_level_context();

    assert(current_token().token_type == TokenType::identifier 
        && "parse_assignment_statement called with current_token that was not an identifier");

    auto identifier = handle_identifier();

    get_token();
    // TODO: change '=' to it's own TokenType
    // !!! in any case assignment operators should live in their own category
    assert(is_assignment_operator(current_token())
        && "assignment statement second token not an assignment operator");
   
    get_token(); // eat '='

    Statement* inner_statement = parse_statement();
    Statement* statement = create_assignment_statement(*ast_store_, identifier, 
        create_definition(inner_statement, is_top_level, inner_statement->location), location);

    log("Finished parsing assignment statement from " + statement->location.to_string(), 
        LogLevel::debug_extra);
    return statement;
}

Statement* ParserLayer1::parse_block_statement() {
    auto location = current_token().location;

    assert(is_block_starter(current_token())
        && "parse_block_statement called with current token that is not a block starter");

    unsigned int indent_at_start = indent_level_;
    unsigned int curly_brace_at_start = curly_brace_level_;

    log("Start parsing block statement", LogLevel::debug_extra);
    
    // determine the block type, i.e. curly-brace or indent
    // must trust the assertion above that other types of tokens won't end up here
    TokenType ending_token;

    switch (current_token().token_type) {
        case TokenType::curly_brace_open:
            ending_token = TokenType::curly_brace_close;
            break;
        case TokenType::indent_block_start:
            ending_token = TokenType::indent_block_end;
            break;
        default:
            return fail_statement("Something went wrong: " + current_token().get_string() + 
                " is not a block starter", current_token().location, true);
    }

    get_token(); // eat start token

    Statement* statement = create_block(*ast_store_, {}, location);
    // fetch the substatements vector
    std::vector<Statement*>* substatements = &std::get<Block>(statement->value);

    while (current_token().token_type != ending_token   ||
            indent_level_ > indent_at_start             || // allow nested blocks
            curly_brace_level_ > curly_brace_at_start      // allow nested blocks
    ) {
        if (current_token().token_type == TokenType::eof)
            return fail_statement("Unexpected eof while parsing block statement", location);

        Statement* substatement = parse_statement();
        // discard empty statements
        if (substatement->statement_type != StatementType::empty)
            substatements->push_back(substatement);
    }

    get_token(); // eat block closer
    
    if (current_token().token_type == TokenType::semicolon)
        get_token(); // eat possible trailing semicolon

    log("Finished parsing block statement from " + statement->location.to_string(), 
        LogLevel::debug_extra);

    if (substatements->size() == 1) {
        // attempt to simplify
        if (!simplify_single_statement_block(statement))
            return fail_statement("Simplification failed", statement->location, true);
    }

    // simplify empty block
    if (substatements->empty()) {
        statement->statement_type = StatementType::empty;
        statement->value = Undefined{};
        return statement;
    }

    return statement;
}

Statement* ParserLayer1::parse_return_statement() {
    auto location = current_token().location;

    assert(current_token().token_type == TokenType::return_t &&
        "Parse_return_statement called with current token other than return");

    get_token(); //eat return
    Expression* expression = parse_expression();
    Statement* statement = create_return_statement(*ast_store_, expression, location);

    assert(is_statement_separator(current_token()) 
        && "Return statement didn't end in statement separator");

    if (current_token().token_type == TokenType::semicolon)
        get_token(); //eat statement separator

    log("Parsed return statement", LogLevel::debug_extra);
    return statement;
}

} // namespace Maps