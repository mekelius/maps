# TODO

## Priorities

#### Needed for 0.1

0.1 focus is on expressions (parse, eval, print)

| Feature                                               | State     | What's missing |
| ---                                                   | ---       | ---            |
| Operator definitions                                  | testing   | parsing        |
| Parsing needs a lot of work                           | in progress | bits and pieces |
| Print expressions and statements in messages nicely   | done      | Callables?     |
| Runtime casts                               | in progress | rest of the compile time |
| Function calls need to work and be smooth             | testing   | partial application |
| Arithmetic stuff working and smooth  | in progress | builtin definitions maps side |
| Blocks and parentheses should work well               | testing   | parsing messy |
| repl improvements                                     | done?     |               |
| clean up IR_Generator                                 | total mess | refactor     |
| restore mapsc functionality                           | ???       | testing       |

#### Accumulated tweaks

- Clean up the responsibilities of converting values
    - now IR_gen does some and earlier passes do some
- Name resolution should return false on fail
- rename TypeRegistry to TypeStore
- Fix pragmas when we do multiple files
- move simplifying blocks and other ast nodes out of layer1
- rename AST to AST_Store
- use vector or array instead of hashmap as TypeMap for efficiency

#### UX tweaks

- aliases
    - :toggle no-eval == :toggle eval
- :no-eval and :eval

## Planned for 0.2

#### Priority features for 0.2 and onward

- if, for etc.
- top-level-evaluation via pragma
- Scopes and contexts (likely 0.2)
- Retain definitions in REPL
- Internal: Rename layers
- Named args
- Traits and generics
- Optional unwrapping syntax

## Planned for later

#### architectural things

- Line-end real-estate
- Combine asts
- Parser gc

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


#### Possible simplifications

- do type arguments need to hold type constructs or should they hold type references?
- operator identifiers and identifiers could be rolled into one, if not for named type args.
    - except, how about operators as type args? would need special syntax