#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef struct List_t List_t;

size_t LexicalAnalysis(const char* s, List_t* tokens);

#endif /* LEXER_H */

// [] - выполняется точно 1 раз
// () - выполняется 0 или 1 раз
// []* - выполняется >= 1 раз
// ()* - выполняется >= 0 раз

/*
Program         := (FuncDec | StatementList) "!END_TOKEN!"

FuncDec         := DataTypes IDENTIFIER "(" (Parameters) ")" [Block | ";"]            //ANCHOR - block done
Parameters      := DataTypes IDENTIFIER ("," DataTypes IDENTIFIER)*                   //ANCHOR - done

StatementList   := [Statement]*
Statement       := VarDec                                                             //ANCHOR - done
                 | Assignment
                 | IfStatement
                 | WhileStatement
                 | ReturnStatement
                 | Block
                 | ExprStatement
                 | Print

IfStatement     := "if" "(" Expression ")" Block                                      //ANCHOR - done
                    ("else if" "(" Expression ")" Block)*
                    ("else" Block)
WhileStatement  := "while" "(" Expression ")" Block                                   //ANCHOR - done
ReturnStatement := "return" Expression ";"                                            //ANCHOR - done

Block           := "{" StatementList "}"                                              //ANCHOR - done
ExprStatement   := Expression ";"                                                     //ANCHOR - done

VarDec          := DataTypes IDENTIFIER ("=" Expression) ";"                          //ANCHOR - done
Assignment      := [IDENTIFIER "="]* Expression ";"                                   //ANCHOR - done

Print           := "print" "(" Expression ")" ";"

Expression      := LogicalOr                                                          //ANCHOR - done

LogicalOr       := LogicalAnd ( "||" LogicalAnd )*                                    //ANCHOR - done
LogicalAnd      := Equality ( "&&" Equality )*                                        //ANCHOR - done

Equality        := Comparison ( ["==" | "!="] Comparison )*                           //ANCHOR - done
Comparison      := Term ( ["<" | ">" | "<=" | ">="] Term )*                           //ANCHOR - done

Term            := Factor ( ['+''-'] Factor )*                                        //ANCHOR - done
Factor          := Primary ( ['*''/'] Primary )*                                      //ANCHOR - done
Primary         := FuncCall                                                           //ANCHOR - done
                 | IDENTIFIER 
                 | NUM 
                 | Input
                 | "true" 
                 | "false" 
                 | '('Expression')'

Input           := "input" "("  ")"                                                   //ANCHOR - done

FuncCall        := IDENTIFIER "(" (Arguments) ")"                                     //ANCHOR - done

Arguments       := Expression ("," Expression)*                                       //ANCHOR - done
DataTypes       := "short" | "int" | "long" | "char" | "void" | "float"               //ANCHOR - done

IDENTIFIER      := TYPE_VARIABLE                                                      //ANCHOR - done
NUM             := TYPE_NUMBER                                                        //ANCHOR - done
*/

// Assignment      := IDENTIFIER ["=" IDENTIFIER| Expression]* ";"