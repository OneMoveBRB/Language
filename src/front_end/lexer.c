#include "../../include/front_end/lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "../../clibs/List/list.h"

#include "../../include/front_end/front_end.h"
#include "../../include/io.h"

#ifdef _WIN32
    #include <string.h>
    #define strncasecmp _strnicmp
#else
    #include <strings.h>
#endif

#define va_arg_enum(type) ((type)va_arg(args, int))

typedef struct TokenTypeMapping {
    const char* string;
    TokenType type;
} TokenTypeMapping;

TokenTypeMapping reference_types[] = {
    {"short" ,   TOKEN_TYPE_SHORT               },
    {"int"   ,   TOKEN_TYPE_INT                 },
    {"long"  ,   TOKEN_TYPE_LONG                },
    {"double",   TOKEN_TYPE_DOUBLE              },
    {"char"  ,   TOKEN_TYPE_CHAR                },
    {"void"  ,   TOKEN_TYPE_VOID                },

    {"true"  ,   TOKEN_TYPE_BOOL_TRUE           },
    {"false" ,   TOKEN_TYPE_BOOL_FALSE          },

    {","     ,   TOKEN_TYPE_COMMA               },
    {";"     ,   TOKEN_TYPE_SEMICOLON           },

    {"("     ,   TOKEN_TYPE_ROUND_BRACKET_OPEN  },
    {")"     ,   TOKEN_TYPE_ROUND_BRACKET_CLOSE },
    {"{"     ,   TOKEN_TYPE_CURLY_BRACKET_OPEN  },
    {"}"     ,   TOKEN_TYPE_CURLY_BRACKET_CLOSE },
    {"["     ,   TOKEN_TYPE_SQUARE_BRACKET_OPEN },
    {"]"     ,   TOKEN_TYPE_SQUARE_BRACKET_CLOSE},

    {"+"     ,   TOKEN_TYPE_BIN_ADD             },
    {"-"     ,   TOKEN_TYPE_BIN_SUB             },
    {"*"     ,   TOKEN_TYPE_BIN_MUL             },
    {"/"     ,   TOKEN_TYPE_BIN_DIV             },
    // {"^",   TOKEN_TYPE_BIN_POW},

    {"<"     ,   TOKEN_TYPE_BIN_LT              },
    {">"     ,   TOKEN_TYPE_BIN_GT              },
    {"<="    ,   TOKEN_TYPE_BIN_LE              },
    {">="    ,   TOKEN_TYPE_BIN_GE              },
    {"=="    ,   TOKEN_TYPE_BIN_EE              },
    {"!="    ,   TOKEN_TYPE_BIN_NE              },

    {"&&"    ,   TOKEN_TYPE_LAND                },
    {"||"    ,   TOKEN_TYPE_LOR                 },

    {"input" ,   TOKEN_TYPE_INPUT               },
    {"print" ,   TOKEN_TYPE_PRINT               },

    {"="     ,   TOKEN_TYPE_ASSIGNMENT          },

    {"if"    ,  TOKEN_TYPE_STATEMENT_IF         },
    {"else"  ,  TOKEN_TYPE_STATEMENT_ELSE       },
    {"while" ,  TOKEN_TYPE_STATEMENT_WHILE      },
    {"break" ,  TOKEN_TYPE_STATEMENT_BREAK      },
    {"return",  TOKEN_TYPE_STATEMENT_RETURN     }
};

const size_t reference_types_size = sizeof(reference_types)/sizeof(TokenTypeMapping);

static ListErr_t ListPushToken(List_t* tokens, TokenType type, ...);

size_t LexicalAnalysis(const char* s, List_t* tokens) {
    assert( s != NULL      );
    assert( tokens != NULL );

    const char* left_ptr = s;

    while (*s != '\0') {

        char flag = 0;

        /* Comments check */

        if (strncmp(s, "//", 2) == 0) {
            while (*s != '\n' && *s != '\0') {
                ++s;
            }
        }

        if (strncmp(s, "/*", 2) == 0) {
            while (strncmp(s, "*/", 2) != 0 && *s != '\0') {
                ++s;
            }
            if (strncmp(s, "*/", 2) == 0) s += 2;
        }

        if (flag) continue;

        /* Keyword check */

        for (size_t i = 0; i < reference_types_size; i++) {
            const char* ref_str = reference_types[i].string;
            size_t ref_str_size = strlen(ref_str);
            if (strncasecmp(s, ref_str, ref_str_size) == 0) {
                ListPushToken(tokens, reference_types[i].type);

                s += ref_str_size;
                flag = 1;
                break;
            }
        }

        if (flag) continue;

        /* Identifier check */

        if (isalpha(*s) || *s == '_') {
            const char* start_ptr = s;
            size_t len = 1;
            ++s;

            while (isalnum(*s) || *s == '_') {
                ++s;
                ++len;
            }

            ListPushToken(tokens, TOKEN_TYPE_VARIABLE, start_ptr, len);

            flag = 1;
        }

        if (flag) continue;

        /* Number check */
        if (isdigit(*s)) {
            const char* start_ptr = s;
            size_t len = 0;

            while (isdigit(*s)) {
                ++s;
                ++len;
            }

            if (*s == '.') {
                ++s;
                ++len;

                while (isdigit(*s)) {
                    ++s;
                    ++len;
                }

                char* endptr = NULL;
                double num = strtod(start_ptr, &endptr);

                if (endptr == s) {
                    ListPushToken(tokens, TOKEN_TYPE_CONST, CONST_TYPE_DOUBLE, num);
                }
            } else {
                int num = 0;
                if (sscanf(start_ptr, "%d", &num) == 1) {
                    ListPushToken(tokens, TOKEN_TYPE_CONST, CONST_TYPE_INT, num);
                }
            }

            flag = 1;
        }

        /* Space check */

        while (isspace(*s)) {
            ++s;
        }
    }

    ListPushToken(tokens, TOKEN_TYPE_END);

    return (size_t)(s - left_ptr);
}

static ListErr_t ListPushToken(List_t* tokens, TokenType type, ...) {
    assert( tokens != NULL );
    assert( type != TOKEN_TYPE_UNDEF );

    Token temp = {type, NULL};

    va_list args;
    va_start(args, type);

    if (type == TOKEN_TYPE_VARIABLE) {
        const char* str = va_arg(args, const char*);
        temp.data.variable = strndup(str, va_arg(args, size_t));

    } else if (type == TOKEN_TYPE_CONST) {
        ConstType const_type = va_arg_enum(ConstType);
        temp.data.constant.type = const_type;

        switch (const_type) {
        case CONST_TYPE_SHORT:
            temp.data.constant.data.short_const = (short)va_arg(args, int); // short
            break;

        case CONST_TYPE_INT:
            temp.data.constant.data.int_const = va_arg(args, int);
            break;
        
        case CONST_TYPE_LONG:
            temp.data.constant.data.long_const = va_arg(args, long);
            break;

        case CONST_TYPE_DOUBLE:
            temp.data.constant.data.double_const = va_arg(args, double);
            break;

        case CONST_TYPE_CHAR:
            temp.data.constant.data.char_const = (char)va_arg(args, int); // char
            break;

        case CONST_TYPE_UNDEFINED:
            assert(0);

        default:
            assert(0);
        }
    }

    va_end(args);

    return ListPushBack(tokens, &temp);
}
