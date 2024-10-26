#include "blang.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include "token.hpp"
#include <iostream>
#include <memory>
#include <vector>

namespace blang {

Blang::Blang() :
    _logger(std::make_shared<Logger>()),
    _lexer(_logger)    
{}

std::shared_ptr<std::vector<char>> Blang::load_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        throw std::runtime_error("failed to open file: " + filename);
    }

    file.seekg(0, std::ios::end);
    auto file_size = file.tellg();

    if (file_size <= 0) {
        throw std::runtime_error("file may be empty!");
    }

    file.seekg(0, std::ios::beg);
    auto file_buffer_ptr = std::make_shared<std::vector<char>>(
        std::vector<char>(static_cast<std::size_t>(file_size))
        );
    file.read(file_buffer_ptr->data(), file_size);

    file.close();

    return file_buffer_ptr;
}

std::shared_ptr<std::vector<char>> Blang::compile(const std::string& filename) {
    auto file_buffer = load_file(filename);

    auto tokens = _lexer.lexTokens(file_buffer);

    if ((tokens->end() - 1)->type == frontend::EOF_TOKEN) {
        tokens->erase(tokens->end() - 1);
    }

    for (auto& token : *tokens) {
        _logger->log(std::make_shared<LexerLog>(token.line, token.type, token.value));
    }

    for (auto& log : _logger->logs()) {
        std::cout << log->to_string() << std::endl;
    }

    auto ret = std::make_shared<std::vector<char>>();

    return ret;
}

}