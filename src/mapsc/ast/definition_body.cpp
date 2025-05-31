#include "definition_body.hh"

#include "mapsc/compilation_state.hh"

namespace Maps {

DefinitionBody::DefinitionBody(DefinitionHeader* header, LetDefinitionValue value)
:header_(header), value_(value) {}

} // namespace Maps