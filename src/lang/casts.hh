#ifndef __CASTS_HH
#define __CASTS_HH

namespace Maps {

class Expression;
class Type;

// take an expression and tries to cast it in place into target type
// argument const value should be used if this is a new expression created from a known const value
// like a literal
bool not_castable(const Type*, Expression*);
bool cast_from_Int(const Type* target_type, Expression* expression);
bool cast_from_Float(const Type* target_type, Expression* expression);
bool cast_from_Number(const Type* target_type, Expression* expression);
bool cast_from_String(const Type* target_type, Expression* expression);
bool cast_from_Boolean(const Type* target_type, Expression* expression);
bool cast_from_NumberLiteral(const Type* target_type, Expression* expression);

} // namespace Maps

#endif