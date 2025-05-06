# Glossary

## General

| Term           | Meaning                                                                       |
| ---            | ---                                                                           |
| mapsc          | maps compiler                                                                 |
| mapsci         | maps compiler, interactive (a JIT compiler and a REPL)                        |
| REPL           | Read-Evaluate-Print Loop <br> interactive programming and testing environment |
| JIT | Just-In-Time compiler <br> a compiler that outputs executable machine code directly into memory on request |
| llvm           | compiler backend/framework used                                               | 

## Types

| Term                          | Meaning                            |
| ---                           | ---                                |
| Absurd | Internal type of typeless expressions <br> If an expression with this type is evaluated, there's a bug in the compiler |
| Hole                          | Type unspecified by the user, to be inferred |
| Void | Special type that's only legal to use as the single parameter or return type of an impure function <br> The plan is to freely convert between impure and pure functions, remains to be seen how that will be handled... |
|                               |                                                           |
| Native type                   | At the moment a synonym for concrete type                 |
| Concrete type       | A type that codegen knows how to deal with, i.e. is mappable to an llvm type |
| Abstract type                 | Type that needs to be concretized before codegen          |
| Concretization                | Stage in the compilation where abstract types are forced into concrete types or the compilation fails |
| Builtin type                  | A hardcoded type                                          |
|||
| Simple type                   | Type that isn't created by a type constructor             |
| Complex type                  | Type that is created by a type constructor                 |
| Type constructor | A mapping that creates new types from existing ones <br> Analogous to a function that acts on types |
| Type construction | Either a simple type or a type constructor together with its arguments <br> Analogous to a function call |
| Type argument                 | A (possibly named) type construction given to a type constructor |
| Type operator                 | Type constructor that uses operator syntax, such as ->  |
| Type declaration              | User-given declaration for a type of an expression/statement/block |
| Binding type declaration      | Type declaration that creates a scope <br> Its named type arguments bind names inside the target expression/statement/block |
|||
| ADT <br> Algebraic Data Type  | Tuples and tagged unions and their combinations <br> Technically functions are also ADT:s but here they are considered distinct |
| Sum type                      | A tagged union type                                            |
| Product type                  | A tuple type                                                   |
|||
| Pure function   | A function that has no side-effects and no data dependencies outside its arguments |
| Impure function | A function with side-effects and/or data dependencies outside its arguments |

## AST

| Term                              | Meaning    |
| ---                               | ---        |
| AST <br> Abstract Syntax Tree     | Representation of the grammatical structure of source code |
| AST Node                          | Currently either an expression, a statement or a Callable |
| Expression                        | Node of an AST that can be reduced to a single value, typed |
| Statement             | Node of the AST that might not be reducible to a value, possibly untyped |
| Builtin                           | A hard-coded typed object a callable can point to |
| Block                 | Statement consisting of 0 or more sub-statements <br> may create a scope |
| Callable                          | A named node in the AST that points to an expression, a statement or a builtin. In case of statements holds a type for the statement, while in case of expressions proxies the expressions type |
| Scope          | Collection of callables limiting their visibility to certain parts of the program |
| Context                           | Nobody knows |
| Pragma                            | Declaration affecting the parsing and compilation process |
| Top-level                         | Space outside functions |
| Termed Expression                 | Expression that parser layer1 isn't able to parse <br> Contains function calls and/or binary operator expressions |
| Literal                           | Constant value given directly by the user |
| Reference                         | Pointer to an object produced by resolving an identifier |

## Syntax

| Term                              | Meaning    |
| ---                               | ---        |
| Tie <br> Tied expression | Binary operator expression without spaces between operands <br> Gets higher precedence |
