#include "symbol_table.hpp"
#include "type.hpp"
#include <memory>

namespace blang {

namespace entities {

Var::~Var() {
    if (Type::is_same(_type, IntType::get())) {
        delete (int32_t*) _content;
    } else if (Type::is_same(_type, CharType::get())) {
        delete (char*) _content;
    } else if (auto array_t = dynamic_cast<ArrayType*>(_type); array_t) {
        if (Type::is_same(array_t->type(), IntType::get())) {
            delete [] (int32_t*) _content;
        } else if (Type::is_same(array_t->type(), CharType::get())) {
            delete [] (char*) _content;
        }
    } else if (auto ptr_t = dynamic_cast<PtrType*>(_type); ptr_t) {
        if (Type::is_same(ptr_t->next(), IntType::get())) {
            delete [] (int32_t**) _content;
        } else if (Type::is_same(ptr_t->next(), CharType::get())) {
            delete [] (char**) _content;
        }
    }
}

uint32_t SymbolTable::_block_counter = 1;

bool SymbolTable::add(std::shared_ptr<Symbol> symbol) {
    if (auto s = get(symbol->ident()); s == nullptr) {
        _symbols.insert({symbol->ident(), symbol});
        return true;
    }

    return false;
}

std::shared_ptr<Symbol> SymbolTable::get(const std::string& ident) {
    if (auto iter = _symbols.find(ident); iter != _symbols.end()) {
        return iter->second;
    }
    
    return nullptr;
}

bool GlobalSymbolTable::addVar(std::shared_ptr<Var> var) {
    return add(var);
}

bool GlobalSymbolTable::addFunc(std::shared_ptr<Func> func) {
    return add(func);
}

std::shared_ptr<Func> GlobalSymbolTable::getFunc(const std::string& ident) {
    return std::dynamic_pointer_cast<Func>(get(ident));
}

std::shared_ptr<Var> GlobalSymbolTable::getVar(const std::string& ident) {
    return std::dynamic_pointer_cast<Var>(get(ident));
}

std::shared_ptr<GlobalSymbolTable> BlockSymbolTable::getGlobal() {
    auto table = _base;
    std::shared_ptr<BlockSymbolTable> block;
    while ((block = std::dynamic_pointer_cast<BlockSymbolTable>(table)) != nullptr) {
        table = block->_base;
    }

    return std::dynamic_pointer_cast<GlobalSymbolTable>(table);
}

bool BlockSymbolTable::addFunc(std::shared_ptr<Func> func) {
    return getGlobal()->addFunc(func);
}

std::shared_ptr<Func> BlockSymbolTable::getFunc(const std::string& ident) {
    return getGlobal()->getFunc(ident);
}

bool BlockSymbolTable::addVar(std::shared_ptr<Var> var) {
    return add(var);
}

std::shared_ptr<Var> BlockSymbolTable::getVar(const std::string& ident) {
    if (auto var = std::dynamic_pointer_cast<Var>(get(ident)); var) {
        return var;
    }
    auto table = _base;
    std::shared_ptr<BlockSymbolTable> block;
    while ((block = std::dynamic_pointer_cast<BlockSymbolTable>(table)) != nullptr) {
        if (auto var = block->getVar(ident); var) {
            return var;
        }
        table = block->_base;
    }

    return std::dynamic_pointer_cast<GlobalSymbolTable>(table)->getVar(ident);
}

}

}