/**
 * @file lexer.hpp
 * @author fyvoid (fyvo1d@outlook.com)
 * @brief Lexer of blang frontend
 * @version 1.0
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef BLANG_LEXER_H
#define BLANG_LEXER_H

#include "token.hpp"
#include "logger.hpp"

#include <memory>
#include <vector>

namespace blang {

namespace frontend {

/**
 * @brief Lexer of blang
 * 
 */
class Lexer {
private:
    uint32_t _pos = 0;
    uint32_t _line = 0;
    std::shared_ptr<std::vector<char>> _buffer;
    std::shared_ptr<Logger> _logger;

    inline bool atEnd() {
        return _pos >= _buffer->size();
    }
    inline void step() {
        _pos++;
    }
    inline char current() {
        return atEnd() ? (*_buffer)[_buffer->size() - 1]
            : (*_buffer)[_pos];
    }
    inline char peak() {
        return atEnd() ? current() : (*_buffer)[_pos + 1];
    }

    Token lexToken();

    std::string lexString();
    std::string lexChar();
    std::string lexNumber();
    std::string lexIdent();
    Token matchKeyword(const std::string& ident);

public:
    Lexer(std::shared_ptr<Logger> logger);
    /**
    * @brief From input(char vector) lex token vector
    * 
    * @param input_buffer 
    * @return std::shared_ptr<std::vector<Token>> 
    */
    std::shared_ptr<std::vector<Token>> lexTokens(std::shared_ptr<std::vector<char>> input_buffer);
};

}

}

#endif