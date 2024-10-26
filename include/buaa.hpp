#ifndef BLANG_BUAA_H
#define BLANG_BUAA_H

#include "token.hpp"

#include <string>

namespace blang {

namespace buaa {

enum ErrorType {
    ERROR_LOGICAL_AND,
    ERROR_LOGICAL_OR,
    ERROR_IDENT_REDEF,
    ERROR_IDENT_UNDEF,
    ERROR_FUNC_PARAM_COUNT_NOT_MATCH,
    ERROR_FUNC_PARAM_TYPE_NOT_MATCH,
    ERROR_VOID_FUNC_RETURN,
    ERROR_FUNC_NO_RETURN,
    ERROR_CONST_MODIFY,
    ERROR_MISSING_SEMICOLON,
    ERROR_MISSING_BRACE,
    ERROR_MISSING_SQUARE,
    ERROR_PRINTF_PARAM_COUNT_NOT_MATCH,
    ERROR_ITER_IDENT_MISUSE,
};

std::string to_string(ErrorType type);

std::string to_buaa_token(frontend::TokenType type);

}

}

#endif