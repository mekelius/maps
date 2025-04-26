The first example program draft in my fancy new programming language called maps
this file explores syntax and semantics for creating variables and basic type casting

```
// This is a comment
/* this is also a comment */
```

Variables are declared with let
```
let f1 = 3.4
let str1 = "asdsad"
```

Optional type declarations like this
```
let i1 = Int 2
```

Non-narrowing type casting like this, looks identical to type declarations
```
let i2 = 1
let f2 = Float i2
let str2 = String 2134
```

Narrowing type-casting not allowed, must use the built in ? (optional) type
```
let i3 = Int f2                 // error
let i1? = Int? f2               // works, (note the ? allowed in names for easier optional naming)
```

Type declaration applies only to the immediadly following name/literal, use ":" to apply to whole expressions
```
let i4 = Int: 2*4
```

Indentation is significant in continuing statements and expressions
```
let str3 = String:
    "can be on a separate line as long as properly indented"
```