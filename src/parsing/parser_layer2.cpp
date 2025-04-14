#include <cassert>

#include "parser_layer2.hh"
#include "name_resolution.hh"

using Logging::log_error, Logging::log_info;

ParserLayer2::ParserLayer2(AST::AST* ast, Pragma::Pragmas* pragmas)
: ast_(ast), pragmas_(pragmas) {
}

void ParserLayer2::run() {
    resolve_identifiers(*ast_);
    // TODO: infer types
    reduce_termed_expressions();
}

void ParserLayer2::reduce_termed_expressions() {
    for (AST::Expression* expression: ast_->unparsed_termed_expressions) {
        reduce_termed_expression(expression);
    }
}

// TODO: mave this to AST
bool is_value_literal(AST::Expression* expression) {
    switch (expression->expression_type) {
        case AST::ExpressionType::string_literal:
        case AST::ExpressionType::numeric_literal:
            return true;
        
        default:
            return false;
    }
}

AST::Expression* ParserLayer2::reduce_termed_expression(AST::Expression* expression) {
    using AST::ExpressionType, AST::Expression;
    
    struct Properties {
        unsigned int precedence;
        AST::Associativity associativity;
        AST::DeferredBool is_arithmetic;
    };
    
    Expression* final_tree = nullptr;

    std::vector<Properties> operator_properties;
    std::vector<Expression*> lhs;
    
    std::vector<Expression*> terms = std::get<AST::TermedExpressionValue>(expression->value);
    for (int i = 1; i < terms.size(); i++) {
        Expression* current_term = terms.at(i);
        Expression* next_term = terms.at(i + 1);

        unsigned int previous_op_precedence = 0; // 0 is a magic value that means no previous precedence

        switch (current_term->expression_type) {
            case ExpressionType::string_literal:
                // two value literals back-to-back not allowed, or not?
                // TODO: write down the goddamn rules
                if (!lhs.empty() && is_value_literal(lhs.back())) {
                    log_error(current_term->location, "unexpected value literal");
                    ast_->declare_invalid();
                    return ast_->create_expression(ExpressionType::syntax_error, current_term->location);
                }

                lhs.push_back(current_term);
                break;
            case ExpressionType::numeric_literal:
                

            case ExpressionType::termed_expression:
            case ExpressionType::identifier:
                if (!lhs.empty()) {
                    switch (lhs.back()->expression_type) {
                        case ExpressionType::identifier:
                            // might be a call or an arg list
                        case ExpressionType::builtin_function:
                            // its a call
                        
                        default:
                    }
                }
            case ExpressionType::builtin_operator:
                if (AST::get_precedence(current_term->type) > previous_op_precedence) {
                    final_tree
                }
            case ExpressionType::builtin_function:
            case ExpressionType::deferred_call:
            case ExpressionType::call:
            case ExpressionType::tie:

            default:
                assert(false && "unhandled expressiontype in reduce_termed_expression");
        }
    }
}