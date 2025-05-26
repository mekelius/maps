#include "statement.hh"

#include <cassert>

#include "mapsc/ast/expression.hh"

namespace Maps {

Statement::Statement(StatementType statement_type, SourceLocation location)
:statement_type(statement_type), location(location) {
    switch (statement_type) {
        case StatementType::compiler_error:
        case StatementType::user_error:
            value = static_cast<std::string>("");
            break;
        case StatementType::deleted:
            assert(false && "why are we creating statements pre-deleted?");
            value = Undefined{};
            break;
        case StatementType::empty:
            value = Undefined{};
            break;               

        case StatementType::expression_statement:
            break;
        case StatementType::block:
            value = Block{};
            break;  
        case StatementType::assignment:
            break;          
        case StatementType::return_:
            break;             
        //case StatementType::if:break;
        //case StatementType::else:break;
        //case StatementType::for:break;
        //case StatementType::for_id:break;
        //case StatementType::do_while:break;
        //case StatementType::do_for:break;
        //case StatementType::while/until: break;
        //case StatementType::switch:break;
    }
}

std::string Statement::log_message_string() const {
    switch (statement_type) {
        case StatementType::deleted:
            return "deleted statement";
        case StatementType::compiler_error:
            return "compiler error";
        case StatementType::user_error:
            return "broken statement";
        case StatementType::empty:
            return "empty statement";
        case StatementType::expression_statement:
            return std::get<Expression*>(value)->log_message_string();

        case StatementType::block:
            return "block";

        case StatementType::assignment:
            return "assignment to " + std::get<Assignment>(value).identifier;

        case StatementType::return_:
            return "return statement";

        // case StatementType::if,
        // case StatementType::else,
            // return "conditional";
        // case StatementType::for,
        // case StatementType::for_in,
        // case StatementType::do_while,
        // case StatementType::do_for,
        // case StatementType::while/until,
            // return "loop";
        // case StatementType::switch:
            // return "switch statement";
    }
}

bool Statement::is_illegal_as_single_statement_block() const {
    switch (statement_type) {
        case StatementType::expression_statement:
        case StatementType::return_:
        case StatementType::user_error:
        case StatementType::compiler_error:
        case StatementType::empty:
        case StatementType::block:
            return false;

        case StatementType::assignment:
            // ???
            return true;

        case StatementType::deleted:
            return true;
    }
}

bool Statement::is_empty() const {
    return statement_type == StatementType::empty;
}

bool Statement::is_definition() const {
    switch (statement_type) {
        case StatementType::assignment:
            return true;

        default:
            return false;
    }
}

std::string_view Statement::statement_type_string() const {
    switch (statement_type) {
            case StatementType::user_error: return "user_error";
            case StatementType::compiler_error: return "compiler_error";
            case StatementType::deleted: return "deleted";
            case StatementType::empty: return "empty";
            case StatementType::expression_statement: return "expression_statement";
            case StatementType::block: return "block";
            case StatementType::assignment: return "assignment";
            case StatementType::return_: return "return_";
            // case StatementType::if: return "if";
            // case StatementType::else: return "else";
            // case StatementType::for: return "for";
            // case StatementType::for_in: return "for_in";
            // case StatementType::do_while: return "do_while";
            // case StatementType::do_for: return "do_for";
            // case StatementType::while: return "while";
            // case StatementType::until: return "until";
            // case StatementType::switch: return "switch";
    }
}

} // namespace AST