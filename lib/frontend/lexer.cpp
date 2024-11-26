#include "lexer.hpp"
#include "buaa.hpp"
#include "logger.hpp"
#include "token.hpp"

#include <cctype>
#include <map>
#include <memory>
#include <vector>

namespace blang {

namespace frontend {

/**
 * @brief Keywords to token enum
 * 
 */
std::map<std::string, TokenType> keyword_map = {
    {"int", INT},
    {"char", CHAR},
    {"void", VOID},
    {"const", CONST},
    {"if", IF},
    {"else", ELSE},
    {"for", FOR},
    {"break", BREAK},
    {"continue", CONTINUE},
    {"return", RETURN},
    {"printf", PRINTF},
    {"getint", GETINT},
    {"getchar", GETCHAR},
    {"main", MAIN},
};

Lexer::Lexer(std::shared_ptr<Logger> logger) {
    _logger = logger;
    _pos = 0;
    _line = 1;
}

/**
 * @brief From input(char vector) lex token vector
 * 
 * @param input_buffer 
 * @return std::shared_ptr<std::vector<Token>> 
 */
std::shared_ptr<std::vector<Token>> Lexer::lexTokens(std::shared_ptr<std::vector<char>> input_buffer) {
    auto tokens = std::make_shared<std::vector<Token>>();
    _buffer = input_buffer;
    _pos = 0;
    _line = 1;

    while (!atEnd()) {
        while ((current() == ' ' || current() == '\t' || current() == '\b') && !atEnd()) {
            step();
        }
        if (current() == '\n' || current() == '\r') {
            if (current() == '\r') step();
            step();
            _line++;
            continue;
        }
        if (atEnd()) {
            break;
        }
        // clear comments
        if (current() == '/') {
            if (peak() == '/') {
                while (peak() != '\n' && peak() != '\r' && !atEnd()) {
                    step();
                }
                if (peak() == '\r') step();
                step();
                continue;
            } else if (peak() == '*') {
                step();
                while (true) {
                    if (current() == '\n' || current() == '\r') {
                        if (current() == '\r') step();
                        _line++;
                    }
                    if (peak() == '*') {
                        step();
                        if (peak() == '/') {
                            step();
                            break;
                        }
                        continue;
                    }
                    step();
                }
                step();
                continue;
            }
        }
        tokens->push_back(lexToken());
        step();
    }

    return tokens;
}

Token Lexer::lexToken() {
    auto ch = current();

    switch (ch) {
        case '+': return {ADD, "+", _line};
        case '-': return {MINUS, "-", _line};
        case '*': return {MULTIPLY, "*", _line};
        case '%': return {MOD, "%", _line};
        case '/': return {DIVIDE, "/", _line,};

        case '!': {
            if (peak() == '=') {
                step();
                return {NOT_EQUAL, "!=", _line};
            }
            return {NOT, "!", _line};
        }

        case '&': {
            if (peak() == '&') {
                step();
            } else {
                _logger->logError(std::make_shared<ErrorLog>(_line, "& error", buaa::ERROR_LOGICAL_AND));
            }
            return {AND, "&&", _line};
        }
        case '|': {
            if (peak() == '|') {
                step();
            } else {
                _logger->logError(std::make_shared<ErrorLog>(_line, "| error", buaa::ERROR_LOGICAL_OR));
            }
            return {OR, "||", _line};
        }

        case '=': {
            if (peak() == '=') {
                step();
                return {EQUAL, "==", _line};
            } else {
                return {ASSIGN, "=", _line};
            }
        }

        case '>': {
            if (peak() == '=') {
                step();
                return {GREATER_EQUAL, ">=", _line};
            } else {
                return {GREATER, ">", _line};
            }
        }
        case '<': {
            if (peak() == '=') {
                step();
                return {LESSER_EQUAL, "<=", _line};
            } else {
                return {LESSER, "<", _line};
            }
        }

        case '(': return {LEFT_BRACE, "(", _line};
        case ')': return {RIGHT_BRACE, ")", _line};
        case '[': return {LEFT_SQUARE, "[", _line};
        case ']': return {RIGHT_SQUARE, "]", _line};
        case '{': return {LEFT_BRAKET, "{", _line};
        case '}': return {RIGHT_BRAKET, "}", _line};
        case ';': return {SEMICOLON, ";", _line};
        case ',': return {COMMA, ",", _line};

        case '"':
            return {STRING, lexString(), _line};

        case '\'':
            return {CHAR_CONSTANT, lexChar(), _line};

        default:
            if (ch >= '0' && ch <= '9') {
                auto number = lexNumber();
                return {INT_CONSTANT, number, _line};
            }

            if (std::isalpha(ch) || ch == '_') {
                auto ident = lexIdent();
                auto match = matchKeyword(ident);
                if (match.type != IDENT) {
                    return match;
                }
                else return {IDENT, ident, _line};
            }

    }

    return {EOF_TOKEN, "", _line};
}

Token Lexer::matchKeyword(const std::string& ident) {
    auto iter = keyword_map.find(ident);
    if (iter != keyword_map.end()) {
        return {iter->second, iter->first, _line};
    } else {
        return {IDENT, ident, _line};
    }
}

std::string Lexer::lexString() {
    std::string str = std::string();
    while (peak() != '"') {
        auto ch = peak();
        str += ch;
        step();
    }
    step(); // consume '"' at string end

    return str;
}

std::string Lexer::lexChar() {
    auto ret = std::string(1, peak());
    char character = peak();
    step();
    if (character == '\\') {
        ret += std::string(1, peak());
        step();
    }
    step();

    return ret;
}

std::string Lexer::lexNumber() {
    std::string str = "";
    str += current();
    while (peak() >= '0' && peak() <= '9') {
        str += peak();
        step();
    }

    return str;
}

std::string Lexer::lexIdent() {
    std::string ret = std::string();
    ret += current();
    while (std::isalnum(peak()) || peak() == '_') {
            ret += peak();
            step();
        }

    return ret;
}

}

}