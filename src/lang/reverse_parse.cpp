#include <cassert>

#include "../logging.hh"
#include "reverse_parse.hh"

unsigned int indent_stack = 0;

std::string linebreak() {
    return "\n" + std::string(indent_stack * REVERSE_PARSE_INDENT_WIDTH, ' ');
}

std::ostream& operator<<(std::ostream& ostream, AST::CallableBody body);
std::ostream& operator<<(std::ostream& ostream, AST::Expression* expression);

std::ostream& operator<<(std::ostream& ostream, AST::Statement* statement) {
    assert(statement && "Reverse parse encountered a nullptr statement");
    ostream << linebreak();

    switch (statement->statement_type) {
        case AST::StatementType::broken:
            ostream << "@broken statement@";
            break;
        case AST::StatementType::illegal:
            ostream << "@illegal statement@";
            break;
        case AST::StatementType::empty:
            break;

        case AST::StatementType::expression_statement:
            ostream << std::get<AST::Expression*>(statement->value);
            break;

        case AST::StatementType::block: {
            ostream << '{';
            indent_stack++;

            for (AST::Statement* substatement: 
                    std::get<AST::Block>(statement->value)) {
                ostream << substatement;
            }

            indent_stack--;
            ostream << linebreak() << '}';
            break;
        }

        case AST::StatementType::let: {
            auto [name, body] = std::get<AST::Let>(statement->value);
            // assume top level identifiers are created by let-statements
            ostream << "let " << name;
            
            // noninitialized
            if (!body.index())
                break;

            ostream << " = " << body;
                
            // for (const AST::Type* arg_type: arg_types) {
            //     ostream << arg_type->name << " -> ";
            // }
            // ostream << return type
            break;
        }
        
        case AST::StatementType::assignment: {
            auto [name, body] = std::get<AST::Assignment>(statement->value);
            ostream << name << " = " << body;
            break;
        }

        case AST::StatementType::return_:
            ostream << "return" 
                    << std::get<AST::Expression*>(statement->value);
            break;
    }

    return ostream << ';';
}

std::ostream& operator<<(std::ostream& ostream, AST::Expression* expression) {
    assert(expression && "Reverse parse encountered a nullptr expression");

    switch (expression->expression_type) {
        case AST::ExpressionType::string_literal:
            return ostream << "\"" << std::get<std::string>(expression->value) << "\"";
        
        case AST::ExpressionType::numeric_literal:
            return ostream << std::get<std::string>(expression->value);

        case AST::ExpressionType::call: {
            auto [callee, args] = expression->call();
            ostream << callee;
            
            for (AST::Expression* arg_expression: args) {
                ostream << ' ' << arg_expression;
            }            

            return ostream;
        }

        case AST::ExpressionType::termed_expression: {
            indent_stack++;
            ostream << linebreak();

            bool pad_left = false;
            for (AST::Expression* term: expression->terms()) {
                if (term->expression_type == AST::ExpressionType::tie) {
                    pad_left = false;
                } else {
                    ostream << (pad_left ? " " : "") << term;
                    pad_left = true; 
                }
            }
            
            indent_stack--;
            return ostream;
        }

        case AST::ExpressionType::builtin_function:
            return ostream << ( REVERSE_PARSE_INCLUDE_DEBUG_INFO ? "/*built-in:*/ " + std::get<std::string>(expression->value) : std::get<std::string>(expression->value) );

        case AST::ExpressionType::identifier:
        case AST::ExpressionType::builtin_operator:
            return ostream << expression->string_value();

        case AST::ExpressionType::not_implemented:
            return ostream << "Expression type not implemented in parser: " + expression->string_value();

        case AST::ExpressionType::tie:
            return REVERSE_PARSE_INCLUDE_DEBUG_INFO ? ostream << "/*-tie-*/" : ostream;

        case AST::ExpressionType::unresolved_identifier:
            return ostream << ( REVERSE_PARSE_INCLUDE_DEBUG_INFO ? "/*unresolved identifier:*/ " + std::get<std::string>(expression->value) : std::get<std::string>(expression->value) );
            
        case AST::ExpressionType::unresolved_operator:
            return ostream << ( REVERSE_PARSE_INCLUDE_DEBUG_INFO ? "/*unresolved operator:*/ " + expression->string_value() : expression->string_value() );

        case AST::ExpressionType::deferred_call:
            return ostream << "Expression type deferred call not implemented in reverse parser";

        case AST::ExpressionType::syntax_error:
            return ostream << "@SYNTAX ERROR@";

        default:
            return ostream << "Expression type not implemented in reverse parser:" << static_cast<int>(expression->expression_type);
    }
}

// reverse-parse expression into the stream
std::ostream& operator<<(std::ostream& ostream, AST::CallableBody body) {
    switch (body.index()) {
        case 0:
            return ostream << "@empty callable body@";

        case 1: // expression
            return ostream << std::get<AST::Expression*>(body);

        case 2: // statement
            return ostream << std::get<AST::Statement*>(body);

        default:
            assert(false && "unhandled callable body type in reverse_parse");
            return ostream;
    }
}

void reverse_parse(AST::AST& ast, std::ostream& ostream) {
    for (auto statement: ast.root_) {
        ostream << statement;
    }
}
