#include <cassert>

#include "mapsc/logging.hh"
#include "reverse_parse.hh"
#include "mapsc/compiler_options.hh"

#include <iostream>

namespace Maps {


ReverseParser::ReverseParser(std::ostream* ostream)
:ReverseParser(ostream, {}) {}

ReverseParser::ReverseParser(std::ostream* ostream, const Options& options)
    :ostream_(ostream), options_(options) {

    if (CompilerOptions::get(CompilerOption::print_all_types) == "true")
        options_.include_all_types = true;
}

void ReverseParser::reset() {
    skipped_initial_linebreak_doubling_ = false;
    indent_stack_ = 0;
}

std::string ReverseParser::linebreak() {
    return "\n" + std::string(indent_stack_ * options_.indent_width, ' ');
}

ReverseParser& ReverseParser::reverse_parse(AST_Store& ast) {
    reset();

    for (auto [name, callable]: ast.globals_->identifiers_in_order_) {
        *this << "let " << name << " = " << callable->body << ";\n\n";
    }

    return *this << ast.root_->body << '\n';
}

ReverseParser& ReverseParser::print_statement(const Statement& statement) {
    if (options_.debug_separators) *this << "$";

    if (skipped_initial_linebreak_doubling_) {
        *this << linebreak();
    } else {
        skipped_initial_linebreak_doubling_ = true;
    }

    switch (statement.statement_type) {
        case StatementType::deleted:
            assert(false && "deleted statement encountered in-tree");
            *this << "@deleted statement@";
            break;
        case StatementType::broken:
            *this << "@broken statement@";
            break;
        case StatementType::illegal:
            *this << "@illegal statement@";
            break;
        case StatementType::empty:
            break;

        case StatementType::expression_statement:
            *this << std::get<Expression*>(statement.value);
            break;

        case StatementType::block: {
            *this << '{';
            indent_stack_++;

            for (Statement* substatement: 
                    std::get<Block>(statement.value)) {
                *this << substatement;
            }

            indent_stack_--;
            *this << linebreak() << '}';
            break;
        }

        case StatementType::let: {
            auto [name, body] = std::get<Let>(statement.value);
            // assume top level identifiers are created by let-statements
            *this << "let " << name;
            
            // noninitialized
            if (!body.index())
                break;

            *this << " = " << body;
                
            // for (const AST::Type* arg_type: arg_types) {
            //     *this << arg_type->name << " -> ";
            // }
            // *this << return type
            break;
        }

        case StatementType::operator_definition: {
            auto [name, arity, body] = std::get<OperatorStatementValue>(statement.value);

            *this << "operator " << name << " = "
                    << (arity == 2 ? "binary" : "unary")
                    << " something something:"
                    << linebreak()
                    << "body here";
            break;
        }
        
        case StatementType::assignment: {
            auto [name, body] = std::get<Assignment>(statement.value);
            *this << name << " = " << body;
            break;
        }

        case StatementType::return_:
            *this << "return" 
                    << std::get<Expression*>(statement.value);
            break;
    }

    return *this << ';';
}

ReverseParser& ReverseParser::print_expression(Expression& expression) {
    if (options_.debug_separators) *this << "£";

    if (options_.include_all_types)
        print_type_declaration(expression);

    switch (expression.expression_type) {
        case ExpressionType::string_literal:
            return *this << "\"" << std::get<std::string>(expression.value) << "\"";
        
        case ExpressionType::numeric_literal:
            return *this << std::get<std::string>(expression.value);

        case ExpressionType::termed_expression: {
            // indent_stack++;
            // *this << linebreak();
            *this << "( ";

            bool pad_left = false;
            for (Expression* term: expression.terms()) {
                *this << (pad_left ? " " : "");
                
                if (options_.include_debug_info)
                    *this << "/*term:*/";

                *this << term;
                pad_left = true; 
            }
            
            // indent_stack--;
            return *this << " )";
        }

        case ExpressionType::operator_reference:
            if (options_.include_debug_info)
                *this << "/*operator-ref:*/ ";
            return *this << expression.reference_value()->name;
        
        case ExpressionType::reference:
        case ExpressionType::type_reference:
        case ExpressionType::type_operator_reference:
        case ExpressionType::type_constructor_reference:
            if (options_.include_debug_info)
                *this << "/*reference to:*/ ";
            return *this << expression.reference_value()->name;

        case ExpressionType::not_implemented:
            return *this << "Expression type not implemented in parser: " + expression.string_value();

        case ExpressionType::identifier:
            if (options_.include_debug_info)
                *this << "/*unresolved identifier:*/ ";
            return *this << std::get<std::string>(expression.value);
            
        case ExpressionType::value:
            if (*expression.type == Int)
                return *this << std::get<maps_Int>(expression.value);
            
            if (*expression.type == Float)
                return *this << std::get<maps_Float>(expression.value);

            if (*expression.type == Boolean)
                return *this << (std::get<bool>(expression.value) ? "true" : "false");

            if (*expression.type == String)
                return *this << std::get<std::string>(expression.value);

            assert(false && "valuetype not implemented in reverse parser");
            return *this;

        case ExpressionType::type_field_name:
            if (options_.include_debug_info)
                *this << "/*type field name:*/ ";
            return *this << expression.string_value();
        case ExpressionType::type_identifier:
        case ExpressionType::type_operator_identifier:
        case ExpressionType::operator_identifier:
            if (options_.include_debug_info)
                *this << "/*unresolved identifier:*/ ";
            return *this << expression.string_value();

        case ExpressionType::syntax_error:
            return *this << "@SYNTAX ERROR@";

        case ExpressionType::type_construct:
            return *this << "@type construct reverse parsing not implemented@";

        case ExpressionType::type_argument: {
            auto [arg, name] = std::get<TypeArgument>(expression.value);
            return *this << arg << " " << (name ? *name : ""); 
        }

        case ExpressionType::call: {
            auto [callee, args] = expression.call_value();

            // print as an operator expression
            if (callee->is_operator() && args.size() <= 2) {
                switch (args.size()) {
                    case 2:
                        return *this << "( " << args.at(0) << " " << callee->name << " " << args.at(1) << " )";

                    case 1:
                        return *this << "( " << callee->name << args.at(0) << " )";
                   
                    case 0:
                        return *this << "(" << callee->name << ")";
                }
            }

            *this << callee->name << '(';
            
            bool first_arg = true;
            for (Expression* arg_expression: args) {
                *this << (first_arg ? "" : ", ") << arg_expression;
                first_arg = false;
            }            

            return *this << ')';
        }

        case ExpressionType::deleted:
            return *this << "@deleted expression@";

        case ExpressionType::missing_arg:
            return *this << "@missing arg@";
    }
}

// reverse-parse expression into the stream
ReverseParser& ReverseParser::print_callable(CallableBody body) {
    switch (body.index()) {
        case 0:
            return *this << "@empty callable body@";

        case 1: // expression
            return *this << *std::get<Expression*>(body);

        case 2: // statement
            return *this << *std::get<Statement*>(body);

        default:
            assert(false && "unhandled callable body type in reverse_parse");
            return *this;
    }
}

ReverseParser& ReverseParser::print_type_declaration(Expression& expression) {
    if (*expression.type == Absurd)
        return *this;
    
    return *this << expression.type->to_string() << " ";
}

} // namespace Maps