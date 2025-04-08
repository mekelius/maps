#include <cassert>

#include "reverse_parse.hh"
#include "config.hh"

unsigned int indent_stack = 0;

// reverse-parse expression into the stream
std::ostream& operator<<(std::ostream& ostream, AST::Expression* expression) {
    assert(expression && "Reverse parse encountered a nullptr expression");

    switch (expression->expression_type) {
        case AST::ExpressionType::string_literal:
            return ostream << "\"" << expression->string_value << "\"";
        
        case AST::ExpressionType::numeric_literal:
            return ostream << expression->string_value;

        case AST::ExpressionType::call: {

            auto [callee, args] = expression->call_expr;
            ostream << callee;
            
            for (AST::Expression* arg_expression: args) {
                ostream << ' ' << arg_expression;
            }            

            return ostream;
        }

        case AST::ExpressionType::termed_expression: {

            indent_stack++;
            ostream << "\n" << std::string(indent_stack * REVERSE_PARSE_INDENT_WIDTH, ' ');

            bool pad_left = false;
            for (AST::Expression* term: expression->terms) {
                if (term->expression_type == AST::ExpressionType::tie) {
                    pad_left = false;
                } else {
                    ostream << (pad_left ? " " : "") << term;
                    pad_left = true; 
                }
            }
            
            ostream << "\n";
            indent_stack--;

            return ostream;
        }

        case AST::ExpressionType::native_function:
            return ostream << ( REVERSE_PARSE_INCLUDE_DEBUG_INFO ? "/*built-in:*/ " + expression->string_value : expression->string_value );

        case AST::ExpressionType::not_implemented:
            return ostream << "Expression type not implemented in parser";

        case AST::ExpressionType::tie:
            return REVERSE_PARSE_INCLUDE_DEBUG_INFO ? ostream << "/*-tie-*/" : ostream;

        case AST::ExpressionType::native_operator:
            return ostream << expression->string_value;            

        case AST::ExpressionType::unresolved_identifier:
            return ostream << ( REVERSE_PARSE_INCLUDE_DEBUG_INFO ? "/*unresolved identifier:*/ " + expression->string_value : expression->string_value );

        case AST::ExpressionType::function_body:
            return ostream << "Expression type function body not implemented in reverse parser";

        case AST::ExpressionType::deferred_call:
            return ostream << "Expression type deferred call not implemented in reverse parser";

        case AST::ExpressionType::syntax_error:
            return ostream << "!!SYNTAX ERROR!!";

        default:
            return ostream << "Expression type not implemented in reverse parser";
    }
}

void reverse_parse(AST::AST& ast, std::ostream& ostream) {
    for (const std::string& identifier: ast.global.identifiers_in_order) {
        AST::Callable* callable = ast.global.identifiers.at(identifier);
        
        const AST::Type* return_type = callable->expression->type;
        std::vector<const AST::Type*> arg_types = callable->arg_types;

        // assume top level identifiers are created by let-statements
        ostream << "let " << identifier;
        
        if (callable->expression->expression_type == AST::ExpressionType::uninitialized_identifier) {
            ostream << "\n\n";
            continue;
        }

        ostream << " = ";
            
        for (const AST::Type* arg_type: arg_types) {
            ostream << arg_type->name << " -> ";
        }

        ostream << return_type->name << ": " << callable->expression << "\n\n";
    }
}
