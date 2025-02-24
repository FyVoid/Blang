/**
 * @file parser.hpp
 * @author fyvoid (fyvo1d@outlook.com)
 * @brief Parser for blang frontend
 * @version 1.0
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef BLANG_PARSER_H
#define BLANG_PARSER_H

#include <cstdint>
#include <map>
#include <memory>
#include <vector>
#include "logger.hpp"
#include "token.hpp"
#include "ast.hpp"
#include "type.hpp"

namespace blang {

namespace frontend {

using namespace entities;

/**
 * @brief Wrapper of a parse try
 * return by tryParse<T> function, parse log stored in buffer
 * 
 */
struct ParseResult {
    std::shared_ptr<AstNode> node;
    std::vector<std::shared_ptr<Log>> log_buffer;
    std::vector<std::shared_ptr<ErrorLog>> error_buffer;
    ParseResult() : node(nullptr), 
        log_buffer(std::vector<std::shared_ptr<Log>>()), 
        error_buffer(std::vector<std::shared_ptr<ErrorLog>>()) {}
    /**
     * @brief Add a log to log buffer
     * 
     * @param log 
     */
    void log(std::shared_ptr<Log> log) {
        log_buffer.push_back(log);
    }
    /**
     * @brief Add an error to error log buffer
     * 
     * @param error 
     */
    void logError(std::shared_ptr<ErrorLog> error) {
        error_buffer.push_back(error);
    }
    /**
     * @brief Try to get a node of type T
     * 
     * @tparam T 
     * @return std::shared_ptr<T> Return nullptr if not contains expected node
     */
    template<typename T>
    std::shared_ptr<T> get() {
        return std::dynamic_pointer_cast<T>(node);
    }
};

/**
 * @brief Parser support for blang
 * 
 */
class Parser {
private:
    std::shared_ptr<Logger> _logger;
    std::shared_ptr<std::vector<Token>> _tokens;

    uint32_t _pos;
    /**
    * @brief espace character definations
    * 
    */
    static const std::map<char, char> _escape_character_map;

    std::string parseString(const std::string in);
    int32_t parseInt(const std::string in);
    char parseChar(const std::string in);
    Op parseUnaryOp(std::shared_ptr<ParseResult> buffer);
    std::tuple<Type*, std::string> parseFuncParam(std::shared_ptr<ParseResult> result);

    const Token& last() { return (*_tokens)[_pos - 1]; }
    bool atEnd() { return _pos >= _tokens->size(); }
    const Token& current() { return atEnd() ? (*_tokens)[_tokens->size() - 1] : (*_tokens)[_pos]; }
    const Token& peak(uint32_t offset = 1) { 
        return _pos + offset >= _tokens->size() ? (*_tokens)[_tokens->size() - 1] : (*_tokens)[_pos + offset]; 
    }
    std::shared_ptr<LexerLog> step() {
        auto ret = std::make_shared<LexerLog>(line(), current().type, current().value);
        if (!atEnd()) _pos++; 
        return ret;
    }
    uint32_t line() { return current().line; }

    std::shared_ptr<ParseResult> parseCompUnit();
    std::shared_ptr<ParseResult> parseMainFuncDef();
    std::shared_ptr<ParseResult> parseFuncDef();
    std::shared_ptr<ParseResult> parseFuncFParams();
    std::shared_ptr<ParseResult> parseBlock();
    std::shared_ptr<ParseResult> parseBlockItem();
    std::shared_ptr<ParseResult> parseStmt();
    std::shared_ptr<ParseResult> parseExpStmt();
    std::shared_ptr<ParseResult> parseBlockStmt();
    std::shared_ptr<ParseResult> parseIfStmt();
    std::shared_ptr<ParseResult> parseForStmt();
    std::shared_ptr<ParseResult> parseReturnStmt();
    std::shared_ptr<ParseResult> parseAssignStmt();
    std::shared_ptr<ParseResult> parsePrintfStmt();
    std::shared_ptr<ParseResult> parseInitVal(bool is_const);
    std::shared_ptr<ParseResult> parseDef(bool is_const);
    std::shared_ptr<ParseResult> parseDecl();
    std::shared_ptr<ParseResult> parseLAndExp();
    std::shared_ptr<ParseResult> parseEqExp();
    std::shared_ptr<ParseResult> parseRelExp();
    std::shared_ptr<ParseResult> parseLOrExp();
    std::shared_ptr<ParseResult> parseCondExp();
    std::shared_ptr<ParseResult> parseMulExp();
    std::shared_ptr<ParseResult> parseAddExp();
    std::shared_ptr<ParseResult> parseExp(bool is_const=false);
    std::shared_ptr<ParseResult> parseFuncRParams();
    std::shared_ptr<ParseResult> parseUnaryExp();
    std::shared_ptr<ParseResult> parsePrimary();
    std::shared_ptr<ParseResult> parseRVal();
    std::shared_ptr<ParseResult> parseLVal();
    std::shared_ptr<ParseResult> parseValue();

    Type* token2Type(Token token);

    bool match(std::initializer_list<TokenType> list);
    /**
     * @brief Check if next token matches token type
     * 
     * @param type Type to match
     * @return true 
     * @return false 
     */
    bool matchNext(TokenType type) { return peak().type == type; }
    /**
     * @brief Check if current token matches token type
     * 
     * @param type Type to match
     * @return true 
     * @return false 
     */
    bool check(TokenType type) { return current().type == type; }

    void update(std::shared_ptr<ParseResult> buffer, std::shared_ptr<ParseResult> result);
    void revert(uint32_t pos) { _pos = pos; }

    using ParseFuncPtr = std::shared_ptr<ParseResult> (Parser::*)();
    using ParseTypeFuncPtr = std::shared_ptr<ParseResult> (Parser::*)(bool);
    /**
    * @brief Try parsing a node, save results to buffer if success, discard any change if fail
    * 
    * @tparam T Node type to parse
    * @param func Parse function
    * @param buffer Parse result buffer
    * @return std::shared_ptr<T> Return nullptr on failure
    */
    template<typename T>
    std::shared_ptr<T> tryParse(ParseFuncPtr func, std::shared_ptr<ParseResult> buffer);
    /**
    * @brief Try parsing a node with constant flag types, save results to buffer on success, dicard any change on failure
    * 
    * @tparam T Node type to parse
    * @param func Parse function
    * @param is_const constant flag
    * @param buffer Parse result buffer
    * @return std::shared_ptr<T> Return nullptr on failure
    */
    template<typename T>
    std::shared_ptr<T> tryParse(ParseTypeFuncPtr func, bool is_const, std::shared_ptr<ParseResult> buffer);
public:
    Parser(std::shared_ptr<Logger> logger);
    /**
    * @brief Parse tokens from lexer to form ast
    * 
    * @param tokens 
    * @return std::shared_ptr<CompNode> 
    */
    std::shared_ptr<CompNode> parse(std::shared_ptr<std::vector<Token>> tokens);
};

}

}

#endif