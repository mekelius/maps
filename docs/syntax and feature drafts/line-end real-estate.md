# Line-end real-estate
***This is a blue-skies draft, the features don't exist yet***

line ends can have special statements that affect the context in some way
they begin with # and a context specifier

```
let value? = get_value?()          #?DEV stderr("Didn't get a value") // give debug output in dev-context

let unwrap_maybe_string_with_default = String? -> String:
    a? -> a                        #? default "missing value"               // in a ? context, the value will be substituted if a missing value is unwrapped

let un
```