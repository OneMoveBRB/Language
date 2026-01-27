# Frontend
includes Lexical and Syntax Analysis

## Grammar
```
// []  - works exactly 1 time
// ()  - works 0 или 1 times
// []* - works >= 1 times
// ()* - works >= 0 times

Program         := (FuncDec | StatementList) "!END_TOKEN!"

FuncDec         := DataTypes IDENTIFIER "(" (Parameters) ")" [Block | ";"]
Parameters      := DataTypes IDENTIFIER ("," DataTypes IDENTIFIER)*

StatementList   := [Statement]*
Statement       := VarDec
                 | Assignment
                 | IfStatement
                 | WhileStatement
                 | ReturnStatement
                 | Block
                 | ExprStatement


IfStatement     := "if" "(" Expression ")" Block
                    ("else if" "(" Expression ")" Block)*
                    ("else" Block)
WhileStatement  := "while" "(" Expression ")" Block
ReturnStatement := "return" Expression ";"

Block           := "{" StatementList "}"
ExprStatement   := Expression ";"

VarDec          := DataTypes IDENTIFIER ("=" Expression) ";"
Assignment      := [IDENTIFIER "="]* Expression ";"

Expression      := LogicalOr

LogicalOr       := LogicalAnd ( "||" LogicalAnd )*
LogicalAnd      := Equality ( "&&" Equality )*

Equality        := Comparison ( ["==" | "!="] Comparison )*
Comparison      := Term ( ["<" | ">" | "<=" | ">="] Term )*

Term            := Factor ( ['+''-'] Factor )*
Factor          := Primary ( ['*''/'] Primary )*
Primary         := FuncCall | IDENTIFIER | NUM | '('E')' | "true" | "false"

FuncCall        := IDENTIFIER ( "(" (Arguments) ")" )

Arguments       := Expression ("," Expression)*
DataTypes       := "short" | "int" | "long" | "char" | "void" | "float"

IDENTIFIER      := TYPE_VARIABLE
NUM             := TYPE_NUMBER
```

# Symbol Table
is the polytree (directed tree) consisting of scopes (nodes). Global scope - the root, last nested scopes - leaves (pointers to them are located in an additional stack).
Each scope consists HashTable of symbols. Branches related to functions declared globally can extend from the root (there can't be nested functions).

![assignment](/img_sym_tab.svg)

# AST standart

## Assignment
```c
x = y = z = expr;
```
![assignment](/ast_standart/assignment.svg)

## Variable Declaration
```c
int x = expr;
```
![vardec1](/ast_standart/vardec1.svg)

```c
int x;
```
![vardec2](/ast_standart/vardec2.svg)

## Functions

### Function Declaration
```c
int func(int a, char b) {...}
```
![function_declaration](/ast_standart/func_dec.svg)

### Function Call
```c
func(args)
```
![function_call](/ast_standart/func_call.svg)

### Return Statement
```c
return expr;
```
![return_statement](/ast_standart/return_statement.svg)

## Loops

### While Statement
```c
while (cond) {
  body;
}
```
![while_statement](/ast_standart/while_statement.svg)

## Conditional operators

### If Statement
```c
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
```c
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
