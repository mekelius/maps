#ifndef __CASTS_HH
#define __CASTS_HH

namespace Maps {

class Type;
struct Expression;

// take a value expression and tries to cast it in place into target type
bool not_castable(const Type*, Expression&);
bool cast_from_Int(const Type* target_type, Expression& expression);
bool cast_from_Float(const Type* target_type, Expression& expression);
bool cast_from_Number(const Type* target_type, Expression& expression);
bool cast_from_String(const Type* target_type, Expression& expression);
bool cast_from_Boolean(const Type* target_type, Expression& expression);
bool cast_from_NumberLiteral(const Type* target_type, Expression& expression);

bool is_concrete(Expression& expression);
bool not_concretizable(Expression& expression);
bool concretize_Number(Expression& expression);
bool concretize_NumberLiteral(Expression& expression);
bool concretize_StringLiteral(Expression& expression);

} // namespace Maps

#endif