#ifndef __EXPRESSION_PROPERTIES_HH
#define __EXPRESSION_PROPERTIES_HH

namespace Maps {

struct Expression;

bool is_partial_call(const Expression&);
bool is_reduced_value(const Expression&);
bool is_literal(const Expression&);
bool is_illegal(const Expression&);
bool is_reference(const Expression&);
bool is_identifier(const Expression&);
bool is_ok_in_layer2(const Expression&);
bool is_ok_in_codegen(const Expression&);
bool is_castable_expression(const Expression&);
bool is_allowed_in_type_declaration(const Expression&);
bool is_constant_value(const Expression&);
bool is_allowed_as_arg(const Expression&);

} // namespace Maps

#endif