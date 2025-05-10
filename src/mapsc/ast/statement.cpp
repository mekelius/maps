#include "statement.hh"

#include <cassert>

#include "mapsc/ast/expression.hh"

namespace Maps {

Statement::Statement(StatementType statement_type, SourceLocation location)
:statement_type(statement_type), location(location) {
    switch (statement_type) {
        case StatementType::broken:
        case StatementType::illegal:
            value = static_cast<std::string>("");
            break;
        case StatementType::deleted:
            assert(false && "why are we creating statements pre-deleted?");
        case StatementType::empty:
            value = std::monostate{};
            break;               

        case StatementType::expression_statement:
            break;
        case StatementType::block:
            value = Block{};
            break;  
        case StatementType::let:
            value = Let{};
            break;
        case StatementType::operator_definition:
            value = OperatorStatementValue{};
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
        case StatementType::broken:
            return "broken statement";
        case StatementType::illegal:
            return "illegal statement";
        case StatementType::empty:
            return "empty statement";
        case StatementType::expression_statement:
            return std::get<Expression*>(value)->log_message_string();

        case StatementType::block:
            return "block";

        case StatementType::let:
            return "let statement";

        case StatementType::operator_definition:
            return "operator definition";

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
        case StatementType::illegal:
        case StatementType::broken:
        case StatementType::empty:
        case StatementType::block:
            return false;

        case StatementType::assignment:
            // ???
            return true;

        case StatementType::operator_definition:
        case StatementType::deleted:
        case StatementType::let:
            return true;
    }
}

} // namespace AST