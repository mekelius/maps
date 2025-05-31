#include "implementation.hh"

#include <cassert>
#include <variant>

#include "mapsc/source.hh"
#include "mapsc/logging.hh"

#include "mapsc/pragma.hh"

#include "mapsc/ast/statement.hh"
#include "mapsc/ast/operator.hh"
#include "mapsc/ast/ast_store.hh"

#include "mapsc/parser/token.hh"

using std::optional, std::nullopt, std::make_unique;


namespace Maps {

using Log = LogInContext<LogContext::layer1>;

Chunk ParserLayer1::parse_top_level_chunk() {
    Statement* statement;

    switch (current_token().token_type) {
        case TokenType::pragma:
            handle_pragma();
            return parse_top_level_chunk();

        case TokenType::eof:
            return std::monostate{};

        case TokenType::let:
            log("Scoping not yet implemented", LogLevel::warning);
                return parse_top_level_let_definition();

            if (current_token().string_value() == "operator")
                return parse_operator_definition();

        default:
            statement = parse_statement();
            if (statement->statement_type != StatementType::empty && !statement->is_definition() &&
                (pragma_store_->check_flag_value("top-level evaluation", statement->location) || 
                    force_top_level_eval_)) {            
                std::get<Block>(std::get<Statement*>(
                    (*result_.top_level_definition)->body())->value)
                        .push_back(statement);
            }
            return statement;
    }
}

DefinitionHeader* ParserLayer1::parse_top_level_let_definition() {
    auto location = current_token().location;
    
    switch (get_token().token_type) {
        case TokenType::identifier: {
                std::string name = current_token().string_value();
                
                // check if name already exists
                if (identifier_exists(name))
                    return fail_definition("Attempting to redefine identifier " + name, location);

                get_token(); // eat the identifier

                if (is_statement_separator(current_token())) {
                    log("parsed let definition declaring \"" + name + "\" with no definition", 
                        LogLevel::debug_extra);
                    
                    get_token(); // eat the semicolon

                    // create an unitialized identifier
                    auto definition = create_undefined_identifier(name, true, location);
                    if (!definition)
                        return fail_definition("Creating undefined identifier failed", location, true);

                    return *definition; //!!!
                }

                if (is_assignment_operator(current_token())) {
                    assert(false && "not implemented");
                    // get_token(); // eat the assignment operator

                    // auto definition = create_definition(name, true, location);

                    // auto scope
                    // push_context(definition);

                    // LetDefinitionValue body = parse_definition_body();

                    // ast_store_->allocate_definition_body(definition, body);
                    // if (!create_identifier(definition))
                    //     return fail_definition("Creating top level identifier failed", location, true);
                    
                    // auto popped_context = pop_context();

                    // // This means a syntax error
                    // if (!popped_context)
                    //     return fail_definition("Creating top level definition body failed", location);

                    // assert(popped_context == definition && "context stack not returned to correct state");

                    // log("Parsed let definition", LogLevel::debug_extra);
                    // return definition;
                }

                get_token();
                return fail_definition(
                    "Unexpected " + current_token().get_string() + ", in let-definition", location);
            }

        case TokenType::operator_t:
            return fail_definition(
                "operator overloading not yet implemented, ignoring", location);

        default:
            return fail_definition(
                "unexpected token: " + current_token().get_string() + " in let definition", location);
    }
}

Operator* ParserLayer1::parse_operator_definition() {
    auto location = current_token().location;

    assert (false && "not implemented");

    // switch (get_token().token_type) {
    //     case TokenType::operator_t: {
    //         std::string op_string = current_token().string_value();

    //         if (identifier_exists(op_string)) 
    //             return fail_definition(
    //                 "operator: " + op_string, location);

    //         get_token();

    //         if (!is_assignment_operator(current_token()))
    //             return fail_definition(
    //                 "unexpected token: " + current_token().get_string() + 
    //                 " in operator statement, expected \"=\"", location);

    //         get_token();

    //         unsigned int arity = current_token().string_value() == "binary" ? 2 : 1;

    //         switch (current_token().token_type) {
    //             case TokenType::binary:
    //                 arity = 2;
    //                 break;
    //             case TokenType::unary: {
    //                 arity = 1;
    //                 get_token();

    //                 Operator::Fixity fixity;

    //                 switch (current_token().token_type) {
    //                     case TokenType::prefix:
    //                         fixity = Operator::Fixity::unary_prefix;
    //                     case TokenType::postfix:
    //                         fixity = Operator::Fixity::unary_postfix;

    //                     default:
    //                         return fail_definition(
    //                             "unexpected token: " + current_token().get_string() + 
    //                             " in unary operator statement, expected \"prefix|postfix\"", 
    //                             current_token().location);
    //                 }
    //             }
    //             default:
    //                 return fail_definition(
    //                     "unexpected token: " + current_token().get_string() + 
    //                     " in operator statement, expected \"unary|binary\"", 
    //                     current_token().location);
    //         }

    //         get_token();

    //         // UNARY OPERATOR
    //         if (arity == 1) {

    //             get_token(); // eat the fixity specifier

    //             DefinitionBody body;
    //             if (is_block_starter(current_token())) {
    //                 body = parse_block_statement();
    //             } else {
    //                 body = parse_expression();
    //             }

    //             // auto definition = ast_store_->allocate_definition(
    //             //     RT_Operator{op_string, body, {fixity}, true, location});
    //             // parse_scope_->create_identifier(definition);
    //             // log("parsed let statement", LogLevel::debug_extra);
    //             // return definition;
    //         }

    //         // BINARY OPERATOR
    //         if (current_token().token_type != TokenType::number)
    //             return fail_definition("unexpected token: " + current_token().get_string() + 
    //                 " in unary operator statement, expected precedence specifier(positive integer)", 
    //                 location);

    //         unsigned int precedence = std::stoi(current_token().string_value());
    //         if (precedence >= Operator::MAX_PRECEDENCE)
    //             return fail_definition("max operator precedence is " + 
    //                 std::to_string(Operator::MAX_PRECEDENCE), location);

    //         get_token(); // eat the precedence specifier

    //         DefinitionBody body;
    //         if (is_block_starter(current_token())) {
    //             body = parse_block_statement();
    //         } else {
    //             body = parse_expression();
    //         }

    //         auto definition = ast_store_->allocate_definition(
    //             Operator{op_string, body, {Operator::Fixity::binary, precedence}, 
    //                     true, location});
    //         parse_scope_->create_identifier(definition);
    //         log("parsed let statement", LogLevel::debug_extra);
    //         return definition;

    //     }
    //     default:
    //         return fail_definition("unexpected token: " + current_token().get_string() + 
    //             " in operator statement", location);     
    // }
}

LetDefinitionValue ParserLayer1::parse_definition_body() {
    return is_block_starter(current_token()) ?
        LetDefinitionValue{parse_block_statement()} : LetDefinitionValue{parse_expression()};
}

Statement* ParserLayer1::parse_inner_let_definition() {
    assert(false && "not implemented");
}

} // namespace Maps
