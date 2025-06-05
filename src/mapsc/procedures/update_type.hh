#ifndef __UPDATE_TYPE_HH
#define __UPDATE_TYPE_HH

namespace Maps {

class DefinitionBody;
struct Expression;
struct Statement;

// Update type from an unkown type to a (hopefully) inferred type
bool update_type(DefinitionBody& definition);
bool update_type(Expression& expression);
bool update_type(Statement& statement);

} // namespace Maps

#endif