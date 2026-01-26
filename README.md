# AST standart

## Assignment
```
x = y = z = expr;
```
![assignment](/ast_standart/assignment.svg)

## Variable Declaration
```
int x = expr;
```
![vardec1](/ast_standart/vardec1.svg)

```
int x;
```
![vardec2](/ast_standart/vardec2.svg)

## Functions

### Function Declaration
```
int func(int a, char b) {...}
```
![function_declaration](/ast_standart/func_dec.svg)

### Function Call
```
func(args)
```
![function_call](/ast_standart/func_call.svg)

### Return Statement
```
return expr;
```
![return_statement](/ast_standart/return_statement.svg)

## Loops

### While Statement
```
while (cond) {
  body;
}
```
![while_statement](/ast_standart/while_statement.svg)

## Conditional operators

### If Statement
```
if (cond) {
  body;
} else if {
  body;
} else {
  body;
}
```
![if_statement](/ast_standart/if_statement.svg)

## Mark Nodes

### ';' - sentinel node
Aim:
* connects statements, parameters, arguments
* marks scope

## Code Example
```
int fib(int n) {
    int a = 1;
    int b = 1;

    int i = 1;
    while (i < n) {
        int temp = b;
        b = a;
        a = temp;
        a = a + b;

        i = i + 1;
    }

    return a;
}

int main() {
    int f = fib(10);

    return 0;
}
```
![example](/ast_standart/example.svg)
