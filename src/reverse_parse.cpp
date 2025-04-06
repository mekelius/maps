#include "reverse_parse.hh"

// reverse-parse expression into the stream
std::ostream& operator<<(std::ostream& ostream, AST::Expression* expression) {
    switch (expression->expression_type) {
        case AST::ExpressionType::string_literal:
            return ostream << "\"" << expression->string_value << "\"";
        
        case AST::ExpressionType::call: {

            auto [callee, args] = expression->call_expr;
            ostream << callee;
            
            for (AST::Expression* arg_expression: args) {
                ostream << ' ' << arg_expression;
            }            

            return ostream;
        }

        case AST::ExpressionType::native_function:
            return ostream << "@{ BUILT-IN FUNCTION: " << expression->string_value << " }@";

        default:
            return ostream << "UNHANDLED EXPRESSION TYPE!!!";
    }
}

void reverse_parse(AST::AST& ast, std::ostream& ostream) {
    for (auto [identifier, callable] : ast.global.identifiers) {
        const AST::Type* return_type = callable->expression->type;
        std::vector<const AST::Type*> arg_types = callable->arg_types;

        // assume top level identifiers are created by let-statements
        ostream << "let " << identifier << " = " << return_type->name;
            
        for (const AST::Type* arg_type: arg_types) {
            ostream << " -> " << arg_type->name;
        }

        ostream << ": " << callable->expression << "\n";
    }
}
