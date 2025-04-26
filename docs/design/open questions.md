- should changing the program in 1 place be able to lead to erors in another place?

example:
mappings being sometimes compiled to enums causes this problem.

##### module A:
```
let enum1 = string -> string
enum1::key = "value"
export enum1
```

##### module B:
```
import A into { enmu1 }
remove enum1::key
```

##### module C:
```
import A into { enum1 }
let value1 = string enum1 // if we allow this, changing the code at B might make the code invalid here
```

maybe enums should be static by default instead of dynamic?