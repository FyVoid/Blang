/**
 * @file token.hpp
 * @author fyvoid (fyvo1d@outlook.com)
 * @brief Lexer token support
 * @version 1.0
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef BLANG_TOKEN_H
#define BLANG_TOKEN_H

#include <string>
namespace blang {

namespace frontend {

/**
 * @brief Token type enum
 * 
 */
enum TokenType {
    // var types
    INT, CHAR, VOID, CONST,

    IDENT, INT_CONSTANT, CHAR_CONSTANT, STRING,

    SEMICOLON, COMMA,

    IF, ELSE, FOR, BREAK, CONTINUE, RETURN, PRINTF,

    ADD, MINUS, MULTIPLY, DIVIDE, MOD,
    ASSIGN, EQUAL, NOT_EQUAL, GREATER, LESSER, GREATER_EQUAL, LESSER_EQUAL,
    NOT, OR, AND,

    LEFT_BRACE, RIGHT_BRACE, LEFT_BRAKET, RIGHT_BRAKET, LEFT_SQUARE, RIGHT_SQUARE,

    GETINT, GETCHAR, MAIN,

    EOF_TOKEN,
};

/**
 * @brief Token struct wrapper
 * 
 */
struct Token {
    TokenType type;
    std::string value;
    uint32_t line;
};

}

}

#endif