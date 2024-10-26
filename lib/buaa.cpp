#include "buaa.hpp"
#include <map>

namespace blang {

namespace buaa {

std::string to_string(ErrorType type) {
    static std::map<ErrorType, std::string> error_map = {
        {ERROR_LOGICAL_AND, "a"},
        {ERROR_LOGICAL_OR, "a"},
        {ERROR_IDENT_REDEF, "b"},
        {ERROR_IDENT_UNDEF, "c"},
        {ERROR_FUNC_PARAM_COUNT_NOT_MATCH, "d"},
        {ERROR_FUNC_PARAM_TYPE_NOT_MATCH, "e"},
        {ERROR_VOID_FUNC_RETURN, "f"},
        {ERROR_FUNC_NO_RETURN, "g"},
        {ERROR_CONST_MODIFY, "h"},
        {ERROR_MISSING_SEMICOLON, "i"},
        {ERROR_MISSING_BRACE, "j"},
        {ERROR_MISSING_SQUARE, "k"},
        {ERROR_PRINTF_PARAM_COUNT_NOT_MATCH, "l"},
        {ERROR_ITER_IDENT_MISUSE, "m"},
    };

    return error_map.at(type);
}

std::string to_buaa_token(frontend::TokenType type) {
    using namespace frontend;
    static std::map<TokenType, std::string> token_map = {
        {INT, "INTTK"}, {CHAR, "CHARTK"}, {VOID, "VOIDTK"}, {CONST, "CONSTTK"},

        {IDENT, "IDENFR"} , {INT_CONSTANT, "INTCON"}, {CHAR_CONSTANT, "CHRCON"}, {STRING, "STRCON"},

        {SEMICOLON, "SEMICN"}, {COMMA, "COMMA"},

        {IF, "IFTK"}, {ELSE, "ELSETK"}, {FOR, "FORTK"}, {BREAK, "BREAKTK"}, {CONTINUE, "CONTINUETK"}, {RETURN, "RETURNTK"}, {PRINTF, "PRINTFTK"},

        {ADD, "PLUS"}, {MINUS, "MINU"}, {MULTIPLY, "MULT"}, {DIVIDE, "DIV"}, {MOD, "MOD"},
        {ASSIGN, "ASSIGN"}, {EQUAL, "EQL"}, {NOT_EQUAL, "NEQ"}, {GREATER, "GRE"}, {LESSER, "LSS"}, {GREATER_EQUAL, "GEQ"}, {LESSER_EQUAL, "LEQ"},
        {NOT, "NOT"}, {OR, "OR"}, {AND, "AND"},

        {LEFT_BRACE, "LPARENT"}, {RIGHT_BRACE, "RPARENT"}, {LEFT_BRAKET, "LBRACE"}, {RIGHT_BRAKET, "RBRACE"}, {LEFT_SQUARE, "LBRACK"}, {RIGHT_SQUARE, "RBRACK"},

        {EOF_TOKEN, "EOF_TOKEN"}, {GETINT, "GETINTTK"}, {GETCHAR, "GETCHARTK"}, {MAIN, "MAINTK"},
    };

    return token_map.at(type);
}

}

}