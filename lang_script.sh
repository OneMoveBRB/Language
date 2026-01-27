#!/bin/bash

list="List/list.c List/list_private.c"
stack="../Stack/src/stack.c ../Stack/src/stack_dump.c"
hash_table="HashTable/src/hash_table.c HashTable/src/hash_table_dump.c"
buffer="../Buffer/src/buffer.c"

front_end="src/front_end/front_end.c src/front_end/lexer.c src/front_end/syntax.c"
ast="src/ast/ast.c src/ast/ast_dump.c"
symbol_table="src/symbol_table/symbol_table.c src/symbol_table/symbol_table_dump.c"
io="src/io.c"

mode_flag="-D _DEBUG"

source="g++ main.c $front_end $ast $symbol_table $io $list $stack $hash_table $buffer -o lang"

flags=" \
$mode_flag -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat \
-Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy    \
-Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op           \
-Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow           \
-Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn                          \
-Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef             \
-Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers                \
-Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector            \
-fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla   \
-fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr \
"

command="$source $flags"

$command
