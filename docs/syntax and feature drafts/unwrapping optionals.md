# Optionals
***This is a blue-skies draft, the features don't exist yet***

Optionals can be unwrapped using the ?= operator
```
let example1 = Float x -> Number?: {
    let intermediate ?= Int x
    return intermediate * 2
}
```

Optional contexts allow for automatic unwrapping
```
let example2 = Number x -> Number y -> Number?: ?{
    let intermediate1 = sqrt(x)
    let intermediate2 = sqrt(y)

    return x + y
}
```

Optionals can also be given a fallback value
```
let example3 = Number? x -> Number? y -> Number: (x ?? 0) + (y ?? 0)
```

Or be given a handler
```
let example4 = HttpResponse response ~> UserData?: {
    return response.body
                |parse_json
                ?.users
                ?['Billy']
                ?.data
} ?? {
    users_with_broken_data += 'Billy'
    return fail
}
```

Line-end real-estate allows for adding context information to optionals failing
```
let example4 = Number x -> Number y -> Number?: ?{
    let intermediate1 = sqrt(x)                         #?DEV stderr( stacktrace() + "arg x < 0" )  
    let intermediate2 = sqrt(y)                         #?DEV stderr( stacktrace() + "arg y < 0" )

    return x + y
}
```

Often unwrapping optionals can be done implicitly
```
let optional1 = Number? x -> Number? y -> Number?: x + y
```

Or just implied with dependencies
```
let optional3 = Number -> Number -> Number?: sqrt(x) + sqrt(y)
```

Monadic unwrapping exists as well
```
let optional4 = Float -> Number?: sqrt >=> Int >=> 100
```

Optionals can be unwrapped by pattern matching, but it's kinda clunky
```
operator / = Number -> Number -> Number?: {
    Number / ( Number != 0 ) -> Number:
        a / b = divide!(a, b)
    Number / 0               -> Fail                    // Fail type has only one value so the function definition can be inferred
}
```

Much cleaner
```
operator divide2 = Number -> Number != 0 -> Number?: divide!(a, b)
```