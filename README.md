# Lambda Calculus Solver

Specify a file containing an expression, `echo -n "(x.(y.xy))(x.x)" > problem.txt`, with `./lambda problem.txt`.

## Grammar

```
expr    = [var] | [func] | [app]
var     = [a-Z]
app     = [expr] [expr]
func    = ( [var] . [expr] )

funcapp = [func] [func]
code    = [funcapp]
```

Examples are shown below.

`(x.x)` is a function with an argument `x`, and an expression body `x`. Further, `(x.(y.xy))` is a function with an argument `x` and a body with a function that accepts an argument `y`. Then, `y` is applied to `x` from the enclosing scope.

## Caveats

Expressions with no normal form will take an indefinite amount of time as the reduction sequence is not terminal (e.g. `(x.xx)(x.xx)`).

## Build Instructions

```
$ git clone https://github.com/birb007/lambda.git
$ cd lambda
$ make
```

A `lambda` executable should be present in the main repository directory.
