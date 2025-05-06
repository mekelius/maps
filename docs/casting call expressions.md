# Casting call expressions

Concretizing call expressions has a number of challenges that bring the focus to unknown values of abstract types in general.

Different scenarios:

| Scenario | solution? | complexity | requires traits | status |
| ---      | ---       | ---        | ---             | ---    |
| A declared type of a value does not match the de facto type |try to cast | simple | no | done
| A value has abstract de-facto type at type concretization step | use type.concretization_function to try to find a concrete type | not bad | no, but can tie in with them | done |
| a call expression has uniform abstract types, and there's no Maps side specialization | find the closest maps side specialization and cast into that? | bad | yes? | - |
| a call expression has non-uniform arguments of disparate types, but specializations are for uniform types | somehow try to find the most loose specialization and try to cast into that | bad | yes? | - |
| a call expression has an abstract declared type, but the arguments only fit a more concrete specialization | ??? | ??? | ??? | - |