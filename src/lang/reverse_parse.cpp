#include <cassert>

#include "../logging.hh"
#include "reverse_parse.hh"

constexpr bool REVERSE_PARSE_INCLUDE_DEBUG_INFO = false;
constexpr bool REVERSE_PARSE_DEBUG_SEPARATORS = false;
constexpr unsigned int REVERSE_PARSE_INDENT_WIDTH = 4;

unsigned int indent_stack = 0;

std::string linebreak() {
    return "\n" + std::string(indent_stack * REVERSE_PARSE_INDENT_WIDTH, ' ');
}

std::ostream& operator<<(std::ostream& ostream, Maps::CallableBody body);
std::ostream& operator<<(std::ostream& ostream, Maps::Expression* expression);

std::ostream& operator<<(std::ostream& ostream, Maps::Statement* statement) {
    if (REVERSE_PARSE_DEBUG_SEPARATORS) ostream << "$";

    assert(statement && "Reverse parse encountered a nullptr statement");
    ostream << linebreak();

    switch (statement->statement_type) {
        case Maps::StatementType::broken:
            ostream << "@broken statement@";
            break;
        case Maps::StatementType::illegal:
            ostream << "@illegal statement@";
            break;
        case Maps::StatementType::empty:
            break;

        case Maps::StatementType::expression_statement:
            ostream << std::get<Maps::Expression*>(statement->value);
            break;

        case Maps::StatementType::block: {
            ostream << '{';
            indent_stack++;

            for (Maps::Statement* substatement: 
                    std::get<Maps::Block>(statement->value)) {
                ostream << substatement;
            }

            indent_stack--;
            ostream << linebreak() << '}';
            break;
        }

        case Maps::StatementType::let: {
            auto [name, body] = std::get<Maps::Let>(statement->value);
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

        case Maps::StatementType::operator_s: {
            auto [name, arity, body] = std::get<Maps::OperatorStatementValue>(statement->value);

            ostream << "operator " << name << " = "
                    << (arity == 2 ? "binary" : "unary")
                    << " something something:"
                    << linebreak()
                    << "body here";
            break;
        }
        
        case Maps::StatementType::assignment: {
            auto [name, body] = std::get<Maps::Assignment>(statement->value);
            ostream << name << " = " << body;
            break;
        }

        case Maps::StatementType::return_:
            ostream << "return" 
                    << std::get<Maps::Expression*>(statement->value);
            break;
    }

    return ostream << ';';
}

std::ostream& operator<<(std::ostream& ostream, Maps::Expression* expression) {
    assert(expression && "Reverse parse encountered a nullptr expression");
    if (REVERSE_PARSE_DEBUG_SEPARATORS) ostream << "£";

    switch (expression->expression_type) {
        case Maps::ExpressionType::string_literal:
            return ostream << "\"" << std::get<std::string>(expression->value) << "\"";
        
        case Maps::ExpressionType::numeric_literal:
            return ostream << std::get<std::string>(expression->value);

        case Maps::ExpressionType::termed_expression: {
            // indent_stack++;
            // ostream << linebreak();
            ostream << "( ";

            bool pad_left = false;
            for (Maps::Expression* term: expression->terms()) {
                ostream << (pad_left ? " " : "");
                
                if (REVERSE_PARSE_INCLUDE_DEBUG_INFO)
                    ostream << "/*term:*/";

                ostream << term;
                pad_left = true; 
            }
            
            // indent_stack--;
            return ostream << " )";
        }

        case Maps::ExpressionType::operator_reference:
            if (REVERSE_PARSE_INCLUDE_DEBUG_INFO)
                ostream << "/*operator-ref:*/ ";
            return ostream << std::get<std::string>(expression->value);
        
        case Maps::ExpressionType::reference:
        case Maps::ExpressionType::type_reference:
        case Maps::ExpressionType::type_operator_reference:
        case Maps::ExpressionType::type_constructor_reference:
            if (REVERSE_PARSE_INCLUDE_DEBUG_INFO)
                ostream << "/*reference to:*/ ";
            return ostream << expression->reference_value()->name;

        case Maps::ExpressionType::not_implemented:
            return ostream << "Expression type not implemented in parser: " + expression->string_value();

        case Maps::ExpressionType::identifier:
            if (REVERSE_PARSE_INCLUDE_DEBUG_INFO)
                ostream << "/*unresolved identifier:*/ ";
            return ostream << std::get<std::string>(expression->value);
            
        case Maps::ExpressionType::value:
            if (*expression->type == Maps::Int)
                return ostream << std::get<int>(expression->value);
            
            if (*expression->type == Maps::Float)
                return ostream << std::get<double>(expression->value);

            if (*expression->type == Maps::Boolean)
                return ostream << (std::get<bool>(expression->value) ? "true" : "false");

            if (*expression->type == Maps::String)
                return ostream << std::get<std::string>(expression->value);

            assert(false && "valuetype not implemented in reverse parser");

        case Maps::ExpressionType::type_identifier:
        case Maps::ExpressionType::type_operator_identifier:
        case Maps::ExpressionType::operator_identifier:
            if (REVERSE_PARSE_INCLUDE_DEBUG_INFO)
                ostream << "/*unresolved identifier:*/ ";
            return ostream << expression->string_value();

        case Maps::ExpressionType::syntax_error:
            return ostream << "@SYNTAX ERROR@";

        case Maps::ExpressionType::type_construct:
            return ostream << "@type construct reverse parsing not implemented@";

        case Maps::ExpressionType::type_argument: {
            auto [arg, name] = std::get<Maps::TypeArgument>(expression->value);
            return ostream << arg << " " << (name ? *name : ""); 
        }

        case Maps::ExpressionType::call: {
            auto [callee, args] = expression->call_value();

            // print as an operator expression
            if (callee->is_operator() && args.size() <= 2) {
                switch (args.size()) {
                    case 2:
                        return ostream << "( " << args.at(0) << " " << callee->name << " " << args.at(1) << " )";

                    case 1:
                        return ostream << "( " << callee->name << args.at(0) << " )";
                   
                    case 0:
                        return ostream << "(" << callee->name << ")";
                }
            }

            ostream << callee->name << '(';
            
            bool first_arg = true;
            for (Maps::Expression* arg_expression: args) {
                ostream << (first_arg ? "" : ", ") << arg_expression;
                first_arg = false;
            }            

            return ostream << ')';
        }

        case Maps::ExpressionType::deleted:
            return ostream << "@deleted expression@";

        case Maps::ExpressionType::missing_arg:
            return ostream << "@missing arg@";
    }
}

// reverse-parse expression into the stream
std::ostream& operator<<(std::ostream& ostream, Maps::CallableBody body) {
    switch (body.index()) {
        case 0:
            return ostream << "@empty callable body@";

        case 1: // expression
            return ostream << std::get<Maps::Expression*>(body);

        case 2: // statement
            return ostream << std::get<Maps::Statement*>(body);

        default:
            assert(false && "unhandled callable body type in reverse_parse");
            return ostream;
    }
}

void reverse_parse(Maps::AST& ast, std::ostream& ostream) {
    ostream << ast.root_->body;
}
