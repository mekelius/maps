#ifndef __TYPE_DECLARATION_HH
#define __TYPE_DECLARATION_HH

#include <vector>

namespace Maps {

class Expression;

// goes through a list of possible binding type declarations, parsing them into
// type declarations if valid
// this needs to be run before name resolution, since otherwise named fields would get
// resolved as identifiers
// after this has been run, there needs to be a step to create local scopes from binding
// declarations
[[nodiscard]] bool check_binding_type_declarations(std::vector<Expression*> possible_BTDs);

[[nodiscard]] bool handle_binding_type_declaration(Expression* type_declaration, Expression* target);
bool is_valid_type_declaration(Expression* expression);

} // namespace Maps

#endif