#include "blang.hpp"
#include "lexer.hpp"
#include "logger.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <fstream>

namespace blang {

Blang::Blang() :
    _logger(std::make_shared<Logger>()),
    _lexer(_logger),
    _parser(_logger),
    _syntax_checker(_logger),
    _ir_generator(_logger)
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

    auto global_table = _syntax_checker.check(comp_unit);

    auto llvm_module = _ir_generator.gen(global_table);

    auto output = llvm_module->to_string();

    std::ofstream ir_out("./llvm_ir.txt");
    ir_out << output;
    ir_out.close();

    // std::ofstream log_out("./symbol.txt");
    // for (auto& log : _logger->syntax_logs()) {
    //     log_out << log->to_string() << std::endl;
    //     if (_logger->errors().empty()) {
    //         std::cout << log->to_string() << std::endl;
    //     }
    // }
    // log_out.close();

    _logger->sortError();
    std::ofstream error_out("./error.txt");
    uint32_t last_line = -1;
    for (auto& error : _logger->errors()) {
        if (last_line == error->line) {
            continue;
        }
        error_out << error->to_string() << std::endl;
        std::cout << error->to_string() << std::endl;
        last_line = error->line;
    }
    error_out.close();

    auto ret = std::make_shared<std::vector<char>>();

    return ret;
}

}