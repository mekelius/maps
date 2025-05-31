#include "definition_body.hh"

namespace Maps {

DefinitionBody::DefinitionBody(DefinitionHeader* header, LetDefinitionValue value)
:header_(header), value_(value) {}

} // namespace Maps