# Casting rules for Maps

v0.1

## Known types

#### Primitives

- Boolean
- Int
- Float
- String

#### Abstract types

- Number 
    - could be Float, Int or a user defined numeric type

#### Special types

- Void 
    - Only allowed as return value or only argument in impure functions

#### Parameterized types

- T? (optional)
    - special value is called missing
- T -> T -> ... (pure function)
- T => T => ... (impure function)

#### Internal types

- NumberLiteral     
    - represents a number that hasn't been interpreted yet
- StringLiteral     
    - this behaves like const string
- Hole              
    - marks a type to (hopefully) be filled in by inference
- Absurd            
    - marks a valueless thing during parsing. If ever encountered in codegen it's a bug (in the compiler)

## Casting rules

These are compile time static casts implemented either

### To and from string

- Number >>> String, String >>> Number?
- Int >>> String, String >>> Int?
- Float >>> String, String >>> Float?
- Boolean >>> String, but not the other way

### Between numbers

- Int >>> Number, Number >>> Int?
- Float >>> Number, Number >>> Float?
- Int >>> Float, Float >>> Int?

### Booleans

- T? >>> Boolean (missing is false, otherwise true)

There's no falsiness of zero, empty string etc. Use explicit conditionals.

### Casting optionals

Applying a cast to an optional first tries casting from the optional. If not possible, instead tries to fmap the cast on the value.

### Casting functions return values

Applying a cast to a function first tries casting the function itself. If not possible, instead tries to fmap the cast on the return value.

### Literals and known valued

if a value is known at compile time the optionality from casting can be discarded by the compiler.

