#ifndef BLANG_LOGGER_H
#define BLANG_LOGGER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "token.hpp"
#include "buaa.hpp"

namespace blang {

enum LogType {
    LOG_ERROR, LOG_LEXER, LOG_PARSER, LOG_SYNTAX,
};

struct Log {
    LogType type;
    uint32_t line;
    const std::string message;
    Log(LogType type, uint32_t line, const std::string& message) :
        type(type), line(line), message(message) {}
    virtual std::string to_string() {
        return "(" + std::to_string(line) + ")" + ":" + message;
    }
};

struct ErrorLog : public Log {
    buaa::ErrorType error;
    ErrorLog(uint32_t line, const std::string& message, buaa::ErrorType error) :
        Log(LOG_ERROR, line, message), error(error) {}
    virtual std::string to_string() {
        return std::to_string(line) + " " + buaa::to_string(error);
    }
};

struct LexerLog : public Log {
    frontend::TokenType token_type;
    LexerLog(uint32_t line, frontend::TokenType type, const std::string& value) :
        Log(LOG_LEXER, line, value), token_type(type) {}
    virtual std::string to_string() {
        auto value_str = message;
        if (token_type == frontend::STRING) value_str = "\"" + value_str + "\"";
        if (token_type == frontend::CHAR_CONSTANT) value_str = "'" + value_str + "'";
        return buaa::to_buaa_token(token_type) + " " + value_str;
    }
};

struct ParserLog : public Log {
    ParserLog(uint32_t line, const std::string& name) :
        Log(LOG_PARSER, line, name) {}
    virtual std::string to_string() {
        return "<" + message + ">";
    }
};

struct SyntaxLog : public Log {
    uint32_t blockn;
    std::string type;
    SyntaxLog(uint32_t line, uint32_t blockn, const std::string& name, const std::string& type) :
        Log(LOG_SYNTAX, line, name), blockn(blockn), type(type) {}
    virtual std::string to_string() {
        return std::to_string(blockn) + " " + message + " " + type;
    }
};

class Logger {
private:
    std::vector<std::shared_ptr<ErrorLog>> _errors;
    std::vector<std::shared_ptr<Log>> _logs;
public:
    Logger();
    std::vector<std::shared_ptr<ErrorLog>>& errors() { return _errors; }
    std::vector<std::shared_ptr<Log>>& logs() { return _logs; }
    std::vector<std::shared_ptr<SyntaxLog>> syntax_logs();
    void sortError();
    void logError(std::shared_ptr<ErrorLog> log);
    void log(std::shared_ptr<Log> log);
};

}

#endif