# TODO

## Priorities

#### Short term

- split declarations/definitions off from statements
- finish dsir
    - definitions
    - calls
    - type casts
- do the ifs in dsir
- do like loops an stuff in dsir
- move inserting to global scope into a separate step after layer1 (layer1 should return like dsir does)
- fix arithmetic etc.
- rename layers to stages and clear up the structure
- then:
    - maybe make expression constructors private?
- then:
    - look into conditionals
    - look into runtime casts
        - runtime casts in our design require both mutable strings and optionals
            - optionals require conditionals
            - optionals require some form of contexts to unwrap

#### Current long term goal

write a cl arg parsing library in maps <br>

Needed features:
    - mappings
    - mutable state
    - loops/map
    - type casts
    - mutable strings
    - possibly hashing
    - structs/records

#### slightly longer term

- come up with an approach to test IR
- need to input some builtins as IR
- need to create a base IR module to not reinsert every time
    - from file
- add multiple ir representations for single maps construct
- REPL GC
- run stuff in jit during compilation

#### Needed for 0.0.1

0.0.1 focus is on expressions (parse, eval, print)

- language features
    - immutable variables
        - substitution
    - type casts and simple type declarations
    - arithmetic expressions
    - function calls (and definitions)
        - pure function inlining as substitution
            - imperative pure function return value expression construction
    - blocks and parentheses
    - if, for etc.
    - polymorphic function definitions
        - picking the right specialization

- overall functionality
    - restore mapsc functionality

- architecture and code quality
    - print callables in messages nicely
    - clean up IR_Generator
    - what to do with ast nodes?
        - how to convert between statements and expressions in-place
        - how to make them constexprable

#### Accumulated tweaks

- clean up the responsibilities of converting values
    - now IR_gen does some and earlier passes do some
- name resolution should return false on fail
- fix pragmas when we do multiple files
- move simplifying blocks and other ast nodes out of layer1
- use vector or array instead of hashmap as TypeMap for efficiency
- expression factory functions are a bit all over the place
    - need factories for the remaining types
    - also for remaining statements

#### UX tweaks

- aliases
- :no-eval and :eval

#### Untested features

- hex and octal numbers

## Planned for 0.0.2

#### Priority features for 0.0.2 and onward

- memory and variables
- strings
- operator definitions
- top-level-evaluation via pragma
- scopes and contexts (likely 0.2)
- retain definitions in REPL
- internal: Rename layers
- named args
- traits and generics
- optional unwrapping syntax

## Planned for later

#### big things

- line-end real-estate
- combine asts
- parser gc

#### 0.0.3 and onward

- ADT:s
- type constructors
- access syntax
- mappings
- mutable values?
- pattern matching, destructuring
- lambdas and first class functions

#### To be designed

- context syntax
- ownership and references
- defining mapping providers
- laziness

#### At some point

- add Int64 (and Int32 as an alias for Int)
- utf-8 support in Lexer

#### Possible simplifications

- do type arguments need to hold type constructs or should they hold type references?
- operator identifiers and identifiers could be rolled into one, if not for named type args.
    - except, how about operators as type args? would need special syntax