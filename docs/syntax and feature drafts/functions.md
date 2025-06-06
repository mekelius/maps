# Functions
***This is a blue-skies draft, the features don't exist yet***

## Design goals
- clearly separate impure code
- functions are mappings
    - even impure functions will often have pure parts
- opt-in monadic semantics (like short circuiting optional assignment)
- should some data-structures behave differently as return values?
- unwrapping return?
- contextful blocks?
- support for pipes

## Draft 

=> marks an impure function
```
let main1 = [String] args ~> ExitCode: {
    print("Hello World!")
    return EXIT_SUCCESS
}
```

Contexts can be inferred, but also declared explicitly
```
let main2 = [String] args ~> ExitCode: IO{
    print("Hello World!")
    return EXIT_SUCCESS
}
```

```
let main3 = Void => Bool:
    print("Hello World!") >> false
```

pure functions can be defined in a number of ways
```
let pure1 = *2

let pure2 = x -> x*2                 

let pure3 = Number -> Number:
    x -> x*2
```

pure functions can use imperative style as well
```
let pure4 = Number x -> Number {
    let y = x*2
    return y
}
```

Return value can be initialized like this, saving a line
```
let pure5 = Number x -> Point p {
    p.x = x
    return p
}
```

