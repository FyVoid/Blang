#include <iostream>
#include "blang.hpp"

using blang::Blang;

int main(int argc, char** argv) {
    auto compiler = Blang();
    compiler.compile("./testfile.txt");
    return 0;
}
