# Type specifiers
***This is a blue-skies draft, the features don't exist yet***

Type specifiers can be written in the actual expressions
They must be uppercase (everything type-related must)

```
let x = Int 67
```

Type specifiers and casting look identical

```
let x = Int 6
let y = String x  // "6"
```

Typecasts cannot be assigned, thus

```
let x = Int
```

means an uninitialized Int value (uninitialized values cannot be used)

Type specifier affects the immediately following term, and they ignore tieing

```
let a = Int
let b = Int

let x = String a*b  // TypeError
```

Use parenthesis or ":" to make the specifier affect the whole expression
```
let x = String: a*b
```

This also means that type specifiers usually don't require parenthesis around them

```
let result = a * Float c
```

Function types can be given like this

```
let f = Int -> Int: *2
```

thin arrow means a pure function, fat arrow means an impure one

```
let print = Printable => Void
```

"**Void**" is a special type that's only allowed as the return type of an impure function, 
or as an only impure argument


## Naming arguments and named return values

Arguments can be named like this

```
let f = Int x -> Int y -> Int:
    x*y
```

A return value can also be named, and then it can be manipulated imperatively

```
let f = Int x -> Int y -> String str:
    str += String x
    str += String y
```

You can also use the keyword return to return (mandatory in impure functions)