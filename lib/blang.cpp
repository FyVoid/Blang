#include "blang.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include "token.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <fstream>

namespace blang {

Blang::Blang() :
    _logger(std::make_shared<Logger>()),
    _lexer(_logger),
    _parser(_logger)   
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

    auto comp_unit = _parser.parse(tokens);

    std::fstream log_out("./parser.txt", std::ios::binary | std::ios::out);
    for (auto& log : _logger->logs()) {
        log_out << log->to_string() << std::endl;
    }
    log_out.close();

    std::fstream error_out("./error.txt", std::ios::binary | std::ios::out);
    for (auto& error : _logger->errors()) {
        error_out << error->to_string() << std::endl;
    }
    error_out.close();

    auto ret = std::make_shared<std::vector<char>>();

    return ret;
}

}