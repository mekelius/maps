#ifndef __TYPE_DECLARATION_HH
#define __TYPE_DECLARATION_HH

#include <vector>

namespace Maps {

class Expression;

// replaces identifiers in binding type declarations with field_name expressions
// this needs to be run before name resolution, since otherwise named fields would get
// resolved as identifiers
// after this has been run, there needs to be a step to create local scopes from binding
// declarations
[[nodiscard]] bool handle_BTD_field_names(std::vector<Expression*>& possible_BTDs);

// only handles termed expressions
[[nodiscard]] bool handle_possible_BTD(Expression* possible_BTD);
bool is_valid_type_declaration(Expression* expression);

} // namespace Maps

#endif