#ifndef __PARSER_LAYER_2_HH
#define __PARSER_LAYER_2_HH

#include <vector>

/**
 * Parser layer 2 is responsible for:
 *  - parsing "termed expressions" i.e. binary and unary operators and access expressions
 *  - parsing mapping literals i.e. lists, enums, dictionaries etc.
 */

namespace Maps {

struct Expression;
class CompilationState;

bool run_layer2(CompilationState& state, Expression* unparsed_termed_expression);
bool run_layer2(CompilationState& state, std::vector<Expression*>& unparsed_termed_expressions);

} // namespace Maps

#endif