# TODO

## Priorities

#### Needed for 0.0.1

0.0.1 focus is on expressions (parse, eval, print)

- language features
    - immutable variables
        - substitution
    - Type casts and simple type declarations
    - Arithmetic expressions
    - Function calls (and definitions)
        - inlining
        - pure function inlining as substitution
            - imperative pure function return value expression construction
    - static typing
        - type checking
        - type concretization
    - Blocks and parentheses
    - if, for etc.
    - polymorphic function definitions
        - picking the right specialization

- overall functionality
    - restore mapsc functionality

- architecture and code quality
    - Print callables in messages nicely
    - move code from callables.cpp to simplify.cpp
    - clean up IR_Generator

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

## Planned for 0.0.2

#### Priority features for 0.0.2 and onward

- memory and variables
- strings
- operator definitions
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

#### 0.0.3 and onward

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

#### At some point

- Add Int64 (and Int32 as an alias for Int)

#### Possible simplifications

- do type arguments need to hold type constructs or should they hold type references?
- operator identifiers and identifiers could be rolled into one, if not for named type args.
    - except, how about operators as type args? would need special syntax