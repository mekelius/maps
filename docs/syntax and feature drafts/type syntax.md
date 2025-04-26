# Creating types
***This is a blue-skies draft, the features don't exist yet***

Types are created like this

```
type TypeA
```

Type aliases are created with let

```
alias TypeA = Int
```

This is a transparent alias

```
let a = TypeA
@type of a is Int@ // true
```

creating an opaque wrapper can be done with
```
type PortNumber = { Int }
```

Then wrapping a variable can be done implicitly or explicitly
```
let a = Int
let port = PortNumber{a}
port = {3}
```

Even implicit conversion is ok if the type is known (how to enforce this stuff in practice?)

```
port = 7
```

Unwrapping doesn't happen implicitly
```
let x = Int 
x = port    // TypeError
```

Unwrapping can be done with generic unary unwrapper

```
let x = Int
x = *port
```

Using an explicit conversion
```
Int port
```

or inside an isomorphism context
```
[PortNumber ~= Int]{
    let x = Int
    x = port
}
```


Creating AST:s, needs generic syntax, maybe like so?
```
type Pair = (lhs, rhs) // named markers
type Pair = Type -> Type + Type
```