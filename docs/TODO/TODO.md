# Long term TODO

## Priorities

#### Short term

- deal with add unary
- move creating callables to a separate step after layer1 (layer1 should return a tree instead)
- then:
    - refactor ir_builtins
    - improve logging a bit
    - maybe make expression constructors private?
- then:
    - look into conditionals
    - look into runtime casts

#### Needed for 0.0.1

0.0.1 focus is on expressions (parse, eval, print)

- language features
    - immutable variables
        - substitution
    - Type casts and simple type declarations
    - Arithmetic expressions
    - Function calls (and definitions)
        - pure function inlining as substitution
            - imperative pure function return value expression construction
    - Blocks and parentheses
    - if, for etc.
    - polymorphic function definitions
        - picking the right specialization

- overall functionality
    - restore mapsc functionality
    - make repl preserve state

- architecture and code quality
    - Print callables in messages nicely
    - clean up IR_Generator
    - what to do with ast nodes?
        - how to convert between statements and expressions in-place

#### Accumulated tweaks

- Clean up the responsibilities of converting values
    - now IR_gen does some and earlier passes do some
- Name resolution should return false on fail
- Fix pragmas when we do multiple files
- move simplifying blocks and other ast nodes out of layer1
- use vector or array instead of hashmap as TypeMap for efficiency
- expression factory functions are a bit all over the place
    - args not in consistent order
    - need factories for the remaining types

#### UX tweaks

- aliases
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

#### big things

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
- utf-8 support in Lexer

#### Possible simplifications

- do type arguments need to hold type constructs or should they hold type references?
- operator identifiers and identifiers could be rolled into one, if not for named type args.
    - except, how about operators as type args? would need special syntax