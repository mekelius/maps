#include <cassert>

#include "mapsc/logging.hh"
#include "reverse_parse.hh"

void ReverseParser::reset() {
    skipped_initial_linebreak_doubling_ = false;
    indent_stack_ = 0;
}

std::string ReverseParser::linebreak() {
    return "\n" + std::string(indent_stack_ * options_.indent_width, ' ');
}

ReverseParser& ReverseParser::reverse_parse(Maps::AST_Store& ast) {
    reset();
    return *this << ast.root_->body << '\n';
}

ReverseParser& ReverseParser::print_statement(const Maps::Statement& statement) {
    if (options_.debug_separators) *this << "$";

    if (skipped_initial_linebreak_doubling_) {
        *this << linebreak();
    } else {
        skipped_initial_linebreak_doubling_ = true;
    }

    switch (statement.statement_type) {
        case Maps::StatementType::deleted:
            assert(false && "deleted statement encountered in-tree");
            *this << "@deleted statement@";
            break;
        case Maps::StatementType::broken:
            *this << "@broken statement@";
            break;
        case Maps::StatementType::illegal:
            *this << "@illegal statement@";
            break;
        case Maps::StatementType::empty:
            break;

        case Maps::StatementType::expression_statement:
            *this << std::get<Maps::Expression*>(statement.value);
            break;

        case Maps::StatementType::block: {
            *this << '{';
            indent_stack_++;

            for (Maps::Statement* substatement: 
                    std::get<Maps::Block>(statement.value)) {
                *this << substatement;
            }

            indent_stack_--;
            *this << linebreak() << '}';
            break;
        }

        case Maps::StatementType::let: {
            auto [name, body] = std::get<Maps::Let>(statement.value);
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

        case Maps::StatementType::operator_definition: {
            auto [name, arity, body] = std::get<Maps::OperatorStatementValue>(statement.value);

            *this << "operator " << name << " = "
                    << (arity == 2 ? "binary" : "unary")
                    << " something something:"
                    << linebreak()
                    << "body here";
            break;
        }
        
        case Maps::StatementType::assignment: {
            auto [name, body] = std::get<Maps::Assignment>(statement.value);
            *this << name << " = " << body;
            break;
        }

        case Maps::StatementType::return_:
            *this << "return" 
                    << std::get<Maps::Expression*>(statement.value);
            break;
    }

    return *this << ';';
}

ReverseParser& ReverseParser::print_expression(Maps::Expression& expression) {
    if (options_.debug_separators) *this << "£";

    switch (expression.expression_type) {
        case Maps::ExpressionType::string_literal:
            return *this << "\"" << std::get<std::string>(expression.value) << "\"";
        
        case Maps::ExpressionType::numeric_literal:
            return *this << std::get<std::string>(expression.value);

        case Maps::ExpressionType::termed_expression: {
            // indent_stack++;
            // *this << linebreak();
            *this << "( ";

            bool pad_left = false;
            for (Maps::Expression* term: expression.terms()) {
                *this << (pad_left ? " " : "");
                
                if (options_.include_debug_info)
                    *this << "/*term:*/";

                *this << term;
                pad_left = true; 
            }
            
            // indent_stack--;
            return *this << " )";
        }

        case Maps::ExpressionType::operator_reference:
            if (options_.include_debug_info)
                *this << "/*operator-ref:*/ ";
            return *this << expression.reference_value()->name;
        
        case Maps::ExpressionType::reference:
        case Maps::ExpressionType::type_reference:
        case Maps::ExpressionType::type_operator_reference:
        case Maps::ExpressionType::type_constructor_reference:
            if (options_.include_debug_info)
                *this << "/*reference to:*/ ";
            return *this << expression.reference_value()->name;

        case Maps::ExpressionType::not_implemented:
            return *this << "Expression type not implemented in parser: " + expression.string_value();

        case Maps::ExpressionType::identifier:
            if (options_.include_debug_info)
                *this << "/*unresolved identifier:*/ ";
            return *this << std::get<std::string>(expression.value);
            
        case Maps::ExpressionType::value:
            if (*expression.type == Maps::Int)
                return *this << std::get<int>(expression.value);
            
            if (*expression.type == Maps::Float)
                return *this << std::get<double>(expression.value);

            if (*expression.type == Maps::Boolean)
                return *this << (std::get<bool>(expression.value) ? "true" : "false");

            if (*expression.type == Maps::String)
                return *this << std::get<std::string>(expression.value);

            assert(false && "valuetype not implemented in reverse parser");
            return *this;

        case Maps::ExpressionType::type_field_name:
            if (options_.include_debug_info)
                *this << "/*type field name:*/ ";
            return *this << expression.string_value();
        case Maps::ExpressionType::type_identifier:
        case Maps::ExpressionType::type_operator_identifier:
        case Maps::ExpressionType::operator_identifier:
            if (options_.include_debug_info)
                *this << "/*unresolved identifier:*/ ";
            return *this << expression.string_value();

        case Maps::ExpressionType::syntax_error:
            return *this << "@SYNTAX ERROR@";

        case Maps::ExpressionType::type_construct:
            return *this << "@type construct reverse parsing not implemented@";

        case Maps::ExpressionType::type_argument: {
            auto [arg, name] = std::get<Maps::TypeArgument>(expression.value);
            return *this << arg << " " << (name ? *name : ""); 
        }

        case Maps::ExpressionType::call: {
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
            for (Maps::Expression* arg_expression: args) {
                *this << (first_arg ? "" : ", ") << arg_expression;
                first_arg = false;
            }            

            return *this << ')';
        }

        case Maps::ExpressionType::deleted:
            return *this << "@deleted expression@";

        case Maps::ExpressionType::missing_arg:
            return *this << "@missing arg@";
    }
}

// reverse-parse expression into the stream
ReverseParser& ReverseParser::print_callable(Maps::CallableBody body) {
    switch (body.index()) {
        case 0:
            return *this << "@empty callable body@";

        case 1: // expression
            return *this << *std::get<Maps::Expression*>(body);

        case 2: // statement
            return *this << *std::get<Maps::Statement*>(body);

        default:
            assert(false && "unhandled callable body type in reverse_parse");
            return *this;
    }
}

