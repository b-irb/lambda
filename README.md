# Lambda Calculus Solver

The solver is capable of β-reduction and primative α-conversion, specify a file containing an expression, `echo -n "(x.(y.xy))(x.x)" > problem.txt`, with `./lambda problem.txt` then a simplification will be attempted.

## Demonstration:

![example usage](https://raw.githubusercontent.com/birb007/lambda/master/demo/demo.png)

The expression `(x.(y.xy(x.(y.y))))(x.(y.x))(y.(x.y))` is simplified.

## Build Instructions

```
$ git clone https://github.com/birb007/lambda.git
$ cd lambda
$ make
```

(to produce the debug build, use `make debug`).

A `lambda` executable should be present in the main repository directory.

## Grammar

```
expr    = [var] | [func] | [app]
var     = [a-Z]
app     = [expr] [expr]
func    = ( [var] . [expr] )

funcapp = [func] [func]
code    = [funcapp]
```

## Explanation

`(x.(y.x))(x.x)` will reduce to `(y.(x.x))` by substituting the right expression into the `x` of the left expression (β-reduction). There are no precedence rules, all expressions are left associative with _equal_ precedence. However, the evaluation of subexpressions prior to encapsulating expressions is functionally equivalent. Several examples of lambda calculus written in the solver grammar are shown below.

```
Identity function: (x.x)
This returns the argument applied, it has a trivial introduction and
elimination rule.

True: (x.(y.x))
Boolean algebra primative.

False: (x.(y.y))
Boolean algebra primative.

If-Expr: (x.(y.(z.xyz)))
By applying a boolean operator (i.e. True or False) we will conditionally
evaluate to the proceeding expressions. For example:

(x.(y.(z.xyz)))(x.(y.x))(x.x)(y.y)
(y.(z.(x.(y.x))yz))(x.x)(y.y)
(z.(x.(y.x))(x.x)z)(y.y)
(x.(y.x))(x.x)(y.y)
(y.(x.x))(y.y)
(x.x)

If we had instead chosen (x.(y.y)), false, we would have reduced to (y.y).
Symbolically, (x.x) and (y.y) are the same but the naming distinction is sufficient for demonstration.
```

## Caveats

Expressions with no normal form will take an indefinite amount of time as the reduction sequence is not terminal (e.g. `(x.xx)(x.xx)`).
