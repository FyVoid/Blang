/**
 * @file blang.hpp
 * @author fyvoid (fyvo1d@outlook.com)
 * @brief Blang main class
 * @version 1.0
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef BLANG_H
#define BLANG_H

#include "ir_generator.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include "optimizer.hpp"
#include "parser.hpp"
#include "syntax_checker.hpp"

#include <memory>
#include <vector>

namespace blang {

using namespace frontend;
using namespace backend;

/**
 * @brief The blang compiler
 * 
 */
class Blang {
private:
    std::shared_ptr<Logger> _logger;
    Lexer _lexer;
    Parser _parser;
    SyntaxChecker _syntax_checker;
    IrGenerator _ir_generator;
    Optimizer _optimizer;
    std::shared_ptr<std::vector<char>> load_file(const std::string& filename);
public:
    Blang();
    std::shared_ptr<std::vector<char>> compile(const std::string& filename);
};

}

#endif