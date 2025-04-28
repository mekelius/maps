# Type declarations

Type declarations as of 0.1 are done inline in expressions.

```
let x = Int 45
let str = Mut String "hello"
let f = Int -> Int -> Int: +
```

Type declarations always affect exactly the following subexpression or term. This is called the target of the type declaration.
```
let x = Float ("23" * 89.2)  // TypeError, can't multiply a String and a Float
let x = Float "23" * 89      // valid, because the string literal can be casted into a Float
```

A type declaration can have named fields that become bound in the target expression
```
let f = Int x -> Int y -> Int:
    sqrt (x^2 + y^2)
```

The return value can be named as well. This is maiinly useful in imperative code.
```
let mean = Int x -> Int y -> Int result:
    result = x + y / 2      // not very useful

let create_listing = [String] items -> String delimiter -> String listing:
    let first = Mut Bool true

    for (item in items)
        if first
            listing += item
            first = false
        else
            listing += delimiter + item

    // named return value is returned automatically

```

Note that if the return value type doesn't have a default value, it needs to be initialized
```
let add_to = Int x -> Int result:
    result += x // error, Int doesn't have a default value

let add_to' = Int x -> Int result:
    result = 56
    result += x // error, Int doesn't have a default value

```

### Which type declarations can have named fields?

In order to have named fields the type declaration has to be "binding".

A type declaration is "binding" if it is either:
- the entire right side of a let-statement
- the first subexpression of any (sub)expression that consist of exactly 2 explicit subexpessions

An explicit subexpression is a subexpression that is either a single terminal, has explicit parentheses, starts or ends at ":", is marked with indentation or is tied (lacks whitespace).

Notably this includes let-statement right hand sides and lambda parameters.

This requirement exists due to parsing disambiguation, and is subject to change if a change in parsing strategy allows for a loosening.
