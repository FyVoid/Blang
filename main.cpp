/**
 * @file main.cpp
 * @author fyvoid (fyvo1d@outlook.com)
 * @brief main function of blang
 * @version 0.1
 * @date 2024-12-14
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "blang.hpp"

using blang::Blang;

int main(int argc, char** argv) {
    auto compiler = Blang();
    compiler.compile("./testfile.txt");
    return 0;
}
