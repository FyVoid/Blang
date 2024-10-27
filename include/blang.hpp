#ifndef BLANG_H
#define BLANG_H

#include "lexer.hpp"
#include "logger.hpp"
#include "parser.hpp"

#include <memory>
#include <vector>

namespace blang {

using namespace frontend;

class Blang {
private:
    std::shared_ptr<Logger> _logger;
    Lexer _lexer;
    Parser _parser;
    std::shared_ptr<std::vector<char>> load_file(const std::string& filename);
public:
    Blang();
    std::shared_ptr<std::vector<char>> compile(const std::string& filename);
};

}

#endif