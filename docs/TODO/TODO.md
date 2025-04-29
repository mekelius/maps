# TODO

## Priorities

#### Needed for 0.1

| Feature                                               | State     | What's missing |
| ---                                                   | ---       | ---            |
| Operator definitions                                  | testing   | parsing        |
| Parsing needs a lot of work                          | in progress | bits and pieces |
| Print expressions and statements in messages nicely   | done | Callables? |
| Runtime casts                               | in progress | rest of the compile time |
| Function calls need to be smooth | testing | partial application rust style at least |
| Arithmetic stuff working smooth  | in progress | builtin definitions maps side |
| Blocks and parentheses should work well               | testing | parsing messy |
| if, for etc.                                          | not started |  |

#### ASAP architectural things

- Line-end real-estate
- Source file/module reference to locations

#### Internal cleanup and stuff

- Combine asts
- Parser gc

#### Accumulated tweaks

- Name resolution should return false on fail

## Planned for 0.2

#### Priority features for 0.2 and onward

- Scopes and contexts (likely 0.2)
- Retain definitions in REPL
- Internal: Rename layers
- Named args
- Traits and generics
- Optional unwrapping syntax

## Planned for later

#### 0.3 and onward

- ADT:s
- Type constructors
- Access syntax
- Mappings
- Mutable values?
- Pattern matching, destructuring
- lambdas and first class functions

#### To be designed

- Context syntax
- Ownership and references
- Defining mapping providers
- Laziness
