#include "reverse_parse.hh"

#include <cassert>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>
#include <string>

#include "mapsc/ast/expression.hh"
#include "mapsc/ast/scope.hh"
#include "mapsc/ast/statement.hh"
#include "mapsc/types/type.hh"
#include "mapsc/types/type_defs.hh"
#include "mapsc/compilation_state.hh"

namespace Maps {

ReverseParser::ReverseParser(std::ostream* ostream)
:ReverseParser(ostream, {}) {}

ReverseParser::ReverseParser(std::ostream* ostream, const Options& options)
    :ostream_(ostream), options_(options) {
}

void ReverseParser::reset() {
    skipped_initial_linebreak_doubling_ = false;
    indent_stack_ = 0;
}

std::string ReverseParser::linebreak() {
    return "\n" + std::string(indent_stack_ * options_.indent_width, ' ');
}

ReverseParser& ReverseParser::reverse_parse(const RT_Scope& scope) {
    reset();

    for (auto definition: scope.identifiers_in_order_) {
        *this << *definition;
    }

    return *this;
}

ReverseParser& ReverseParser::print_statement(const Statement& statement) {
    if (skipped_initial_linebreak_doubling_ && statement.statement_type != StatementType::block) {
        *this << linebreak();
    } else {
        skipped_initial_linebreak_doubling_ = true;
    }

    if (options_.debug_separators) 
        *this << "$";

    switch (statement.statement_type) {
        case StatementType::deleted:
            assert(false && "deleted statement encountered in-tree");
            *this << "@deleted statement@";
            break;
        case StatementType::compiler_error:
            *this << "@compiler error@";
            break;
        case StatementType::user_error:
            *this << "@broken statement@";
            break;
        case StatementType::empty:
            break;

        case StatementType::expression_statement:
            *this << *std::get<Expression*>(statement.value);
            break;

        case StatementType::block: {
            *this << '{';
            indent_stack_++;

            for (Statement* substatement: 
                    std::get<Block>(statement.value)) {
                *this << *substatement;
            }

            indent_stack_--;
            *this << linebreak() << '}';
            break;
        }
        
        case StatementType::assignment: {
            auto [name, body] = std::get<Assignment>(statement.value);
            assert(false && "reverse parsing assignments not implemented");

            // *this << name << " = " << body;
            break;
        }

        case StatementType::return_:
            *this << "return" 
                    << *std::get<Expression*>(statement.value);
            break;
    }

    return *this << ';';
}

ReverseParser& ReverseParser::print_expression(const Expression& expression) {
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
            for (Expression* term: std::get<TermedExpressionValue>(expression.value).terms) {
                *this << (pad_left ? " " : "");
                
                if (options_.include_debug_info)
                    *this << "/*term:*/";

                *this << *term;
                pad_left = true; 
            }
            
            // indent_stack--;
            return *this << " )";
        }

        case ExpressionType::prefix_operator_reference:
        case ExpressionType::postfix_operator_reference:
        case ExpressionType::binary_operator_reference:
            if (options_.include_debug_info)
                *this << "/*operator-ref:*/ ";
            return *this << expression.reference_value()->name_string();
        
        case ExpressionType::type_operator_reference:
            return *this << "@type operator reference"; 
        case ExpressionType::type_constructor_reference:
            return *this << "@type constructor reference@"; 

        case ExpressionType::type_reference:
            if (options_.include_debug_info)
                *this << "/*type reference to:*/ ";            
            return *this << std::get<const Type*>(expression.value)->name_string();

        case ExpressionType::reference:
        case ExpressionType::known_value_reference:
            if (options_.include_debug_info)
                *this << "/*reference to:*/ ";
            return *this << expression.reference_value()->name_string();

        case ExpressionType::identifier:
            if (options_.include_debug_info)
                *this << "/*unresolved identifier:*/ ";
            return *this << std::get<std::string>(expression.value);
            
        case ExpressionType::known_value:
            if (*expression.type == Int)
                return *this << std::get<maps_Int>(expression.value);
            
            if (*expression.type == Float)
                return *this << std::get<maps_Float>(expression.value);

            if (*expression.type == Boolean)
                return *this << (std::get<bool>(expression.value) ? "true" : "false");

            if (*expression.type == String)
                return *this << '"' << std::get<std::string>(expression.value) << '"';

            return *this << "@known value of unhandled type \"" << expression.type->name_string() << "\"@"; 

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

        case ExpressionType::user_error:
            return *this << "@SYNTAX ERROR@";

        case ExpressionType::type_construct:
            return *this << "@type construct reverse parsing not implemented@";

        case ExpressionType::type_argument: {
            auto [arg, name] = std::get<TypeArgument>(expression.value);
            return *this << *arg << " " << (name ? *name : ""); 
        }

        case ExpressionType::partial_binop_call_left:
        case ExpressionType::partial_binop_call_right:
        case ExpressionType::partial_call:
        case ExpressionType::call: {
            auto [callee, args] = std::get<CallExpressionValue>(expression.value);

            // print as an operator expression
            if (callee->is_operator() && args.size() <= 2) {
                switch (args.size()) {
                    case 2:
                        return *this << "( " 
                                    << *args.at(0) 
                                    << " " 
                                    << callee->name_string()
                                    << " " 
                                    << *args.at(1) 
                                    << " )";

                    case 1:
                        return *this << "( " << callee->name_string() << *args.at(0) << " )";
                   
                    case 0:
                        return *this << "(" << callee->name_string() << ")";
                }
            }

            *this << callee->name_string() << '(';
            
            bool first_arg = true;
            for (Expression* arg_expression: args) {
                *this << (first_arg ? "" : ", ") << *arg_expression;
                first_arg = false;
            }            

            return *this << ')';
        }

        case ExpressionType::partially_applied_minus:
            return *this << "(-" << *std::get<Expression*>(expression.value) << ")";

        case ExpressionType::minus_sign:
            return *this << "-";

        case ExpressionType::lambda:
            return *this << "\\" << expression.lambda_value().parameters 
                         << ( expression.type->is_pure() ? "-> " : "=> " ) 
                         << expression.lambda_value().body;

        case ExpressionType::ternary_expression:
            return *this << *expression.ternary_value().condition << "?" 
                         << *expression.ternary_value().success << ":" 
                         << *expression.ternary_value().failure;

        case ExpressionType::compiler_error:
            return *this << "@compiler error@";

        case ExpressionType::deleted:
            return *this << "@deleted expression@";

        case ExpressionType::missing_arg:
            return *this << "@missing arg@";

        case ExpressionType::partial_binop_call_both:
            assert(false && "not implemented");
    }
}

ReverseParser& ReverseParser::print_definition(const Definition& definition) {
    auto name = definition.name_string();

    return std::visit(overloaded {
        [this, name](Error body) -> ReverseParser& {
            *this << "\n";
            return print_const_definition_body(body);
        },
        [this, name](External) -> ReverseParser& {
            return *this << "\n@external " << name << "@\n";
        },
        [this, name](BTD_Binding body) -> ReverseParser& {
            auto type = body.type;
            return *this << (*type != Hole ? type->name_string() : "") << name << " ";
        },
        [this, name](Undefined)-> ReverseParser& {
            return *this << "\nlet " << name << "\n";
        },
        [this, name](const Expression* expression)-> ReverseParser& {
            return *this << "\nlet " << name << " = " << *expression << "\n";
        },
        [this, name](const Statement* statement)-> ReverseParser& {
            return *this << "\nlet " << name << " = " << *statement << "\n";
        },
    }, definition.const_body());
}

// reverse-parse expression into the stream
ReverseParser& ReverseParser::print_definition_body(DefinitionBody body) {
    return *this << std::visit( [](auto body) { return const_DefinitionBody{body}; }, body );
}

// reverse-parse expression into the stream
ReverseParser& ReverseParser::print_const_definition_body(const_DefinitionBody body) {
    return std::visit(overloaded {
        [this](Error) -> ReverseParser& {
            return *this << "@error@";
        },
        [this](External) -> ReverseParser& {
            return *this << "@external@";
        },
        [this](BTD_Binding body) -> ReverseParser& {
            auto type = body.type;
            return *this << (*type != Hole ? type->name_string() : "") << "@binding@";
        },
        [this](Undefined)-> ReverseParser& {
            return *this << "@undefined@";
        },
        [this](const Expression* expression)-> ReverseParser& {
            return *this << *expression;
        },
        [this](const Statement* statement)-> ReverseParser& {
            return *this << *statement;
        },
    }, body);
}

ReverseParser& ReverseParser::print_type_declaration(const Expression& expression) {
    if (*expression.type == Absurd)
        return *this;
    
    return *this << expression.type->name_string() << " ";
}

ReverseParser& ReverseParser::print_parameter_list(const ParameterList& parameters) {
    for (auto parameter: parameters)
        *this << *parameter;

    return *this;
}


} // namespace Maps