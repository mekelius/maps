# Termed expression edgecases

| Expression          | Should be parsed into        | Test written? |
| ---                 | ---                          | ---           |
| ```a +b```          | ```a + b```                  | yes           |
| ```a -b```          | ```a - b```                  | no            |
| ```a- b```          | ```a - b```                  | no            |
| ```-(a + b)```      | ```- ( a + b )```            | no            |
| ```(-a + -b) + c``` | ```( (-a) + (-b) ) + c```    | no            |
| ```a - - b```       | ```a - (-b)```               | no            |
| ```a* - b```        | ```a * (-b)```               | no            |
| ```-a*b```          | ```(-a) * b```               | no            |
| ```-a+ -b```        | ```(-a) + (-b)```            | no            |
| ```a *b+c``` <br> ```*b+c``` | ```( a * b ) + c``` since <br> ```\a -> a*b+c``` | no <br> no |
| ```a *b+c * d```    | ```( ( a * b ) + c ) * d```  | no            |
| ```a (* b) + c```   | ```( a * b ) + c```          | no            |
| ```a * b+c* d```    | ```a * ( b + ( c * d ) )```  | no            |
| ```a * b+c+ d```    | ```a * ( b + ( c + d ) )```  | no            | 
| ```a * -b+c+ d```   | ```a * ( -b + ( c + d ) )``` | no            | 
