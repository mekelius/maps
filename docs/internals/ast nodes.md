# AST nodes

v0.1

## Node Types

Currently there are 3 types of AST nodes:
- expressions
- statements
- callables

In short, expressions are things that reduce to a single value and always have a type, while statements represent imperative constructs that may or may not result in a value, and don't always have a type. Callables are typed constructs that have a body that might be an expression, a statement or a builtin (in the future also externals other than builtins).

All AST nodes also carry their location in the source code.

## Expressions

Expressions have a data type, an expression type and a value determined by their expression type. Expressions are separated into 2 "layers". The first layer uses recursive descent to parse most of the tree, but leaves certain expressions called "termed expressions" as lists of terms (really trees). Layer "1.5" is where name resolution and type inference happens (maybe should call it layer 2). Layer 2 then uses recursive ascent to parse the termed expressions. The expression types are:

| Expression type     | lifetime from | to  | Value                               | Status  |
| ---                 | ---           | --- | ---                                 | ---     |
| identifier          | 1 | 1.5             | String                              | done    |
| operator_identifier | 1 | 1.5             | String                              | done    |
| type_operator_identifier | 1 | 1.5        | String                              | work in progress |
| type_identifier     | 1 | 1.5             | String                              | done    |
| termed_expression   | 1   | 2       | [ Expression* ]                            | done    |
| string_literal      | 1   | codegen | String                                     | done    |
| numeric_literal     | 1   | codegen | String                                     | done    |
| reference           | 1.5 | 2       | Callable*                                  | done    |
| operator_reference  | 1.5 | 2       | Callable*                                  | done    |
| type_reference      | 1.5 | 2       | (const) Type*                              | done    |
| type_constructor_reference | 1.5 | 2       | (const) Type*                       | done    |
| type_construct      | 2 | 2  | type_identifier \| ( type_constructor_identifier, [ type_argument ] )                                        | work in progress |
| type_argument       | 2 | 2 | ( type_construct, string? )                | work in progress |
| missing_arg         | 2 | 2 | -                                                  | done    |
| deferred_call       | 2 | 2       | ( Expression*, [ Expression* ] )             | planned |
| call                | 2 (should be 1) | codegen | ( Callable*, [ Expression* ] ) | done    | 
| value               | 2 (1?)          | codegen | ???                            | planned |
| deleted             | - | -       | -                                            | done    | 
| syntax_error        | - | -       | -                                            | done    |
| not_implemented     | - | -       | -                                            | done    |

## Statements

Statements just have a statement type and a value

| Statement type       | Value                                                      | Status  |
| ---                  | ---                                                        | ---     |
| empty                | -                                                          | done    |
| expression statement | Expression*                                                | done    |
| block                | should be: [ Statement* ]                                  | done as a struct |
| let                  | should be: ( string, CallableBody )                        | done as a struct |
| assignment           | should be: ( string, CallableBody )                        | done as a struct |
| return_              | Expression*                                                | done    |
| if                   | ( Expression*, Statement* )                                | planned |
| if_else              | ( [ Expression*, Statement* ], Statement* )                | planned |
| for/do_for           | ( Statement*, Expression*, Statement*, Statement*, bool )  | planned |
| for_in               | ( string, Expression*, Statement* )                        | planned |
| do/while             | ( Expression*, Statement*, bool )                          | planned |
| switch               | ( Expression*, [ ( ???, Statement* ) ], Statement* )       | planned |
| match                | ???                                                        | planned |
| deleted              | -                                                          | done    | 
| broken               | -                                                          | done    |
| illegal              | -                                                          | done    |
| operator_s?          | ???                                                        | ???     |

## Callables

Callables have a name (that might be generated internally), a body that can be either Statement*, Expression* or a Builtin*, and a type. If the body is an expression, the type is proxied from the expression. Callables also optionally store an Operator* that is used to store things like precedence.