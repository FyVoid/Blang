#ifndef BLANG_SYMBOL_TABLE_H
#define BLANG_SYMBOL_TABLE_H

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include "ast.hpp"
#include "ir.hpp"
#include "type.hpp"

namespace blang {

namespace entities {

class Symbol {
protected:
    const std::string _ident;
    Symbol(const std::string& ident) : _ident(ident) {}
    virtual ~Symbol() {}
public:
    std::string ident() { return _ident; }
};

class Var : public Symbol {
private:
    Type* const _type;
    const bool _is_const;
    std::shared_ptr<PtrValue> _value;
    void* _content;
    Var(Type* type, const std::string& ident, bool is_const) :
        Symbol(ident), _type(type), _is_const(is_const) {}
    template<typename T>
    static std::shared_ptr<Var> getSingle(Type* type, const std::string& ident, bool is_const, T val) {
        auto ret = std::shared_ptr<Var>(new Var(type, ident, is_const));
        ret->_content = (void*) new T(val);
        return ret;
    }
    template<typename T>
    static std::shared_ptr<Var> getArray(Type* type, const std::string& ident, bool is_const, uint32_t length) {
        auto ret = std::shared_ptr<Var>(new Var(ArrayType::get(type, length), ident, is_const));
        ret->_content = (void*) new T[length];
        memset(ret->_content, 0, sizeof(T) * length);
        return ret;
    }
    template<typename T>
    static std::shared_ptr<Var> getPtr(Type* type, const std::string& ident, bool is_const) {
        auto ret = std::shared_ptr<Var>(new Var(PtrType::get(type), ident, is_const));
        ret->_content = (void*) new T*(0);
        return ret;
    }
public:
    ~Var();
    static std::shared_ptr<Var> getInt(const std::string& ident, bool is_const, int32_t val=0) {
        return getSingle<int32_t>(IntType::get(), ident, is_const, val);
    }
    static std::shared_ptr<Var> getChar(const std::string& ident, bool is_const, char val=0) {
        return getSingle<char>(CharType::get(), ident, is_const, val);
    }
    static std::shared_ptr<Var> getIntArray(const std::string& ident, bool is_const, uint32_t length) {
        return getArray<int32_t>(IntType::get(), ident, is_const, length);
    }
    static std::shared_ptr<Var> getCharArray(const std::string& ident, bool is_const, uint32_t length) {
        return getArray<char>(CharType::get(), ident, is_const, length);
    }
    static std::shared_ptr<Var> getIntPtr(const std::string& ident, bool is_const) {
        return getPtr<int32_t>(IntType::get(), ident, is_const);
    }
    static std::shared_ptr<Var> getCharPtr(const std::string& ident, bool is_const) {
        return getPtr<char>(CharType::get(), ident, is_const);
    }

    Type* type() { return _type; }
    bool is_const() { return _is_const; }
    template<typename T>
    T* get() { return static_cast<T*>(_content); }
    std::shared_ptr<PtrValue>& value() { return _value; }
};

class Func : public Symbol {
private:
    Type* const _return_type;
    const std::vector<std::tuple<Type*, std::string>> _params;
    std::shared_ptr<Function> _function;
    FuncDefNode* const _node;
public:
    Func(Type* return_type, const std::string& ident, std::vector<std::tuple<Type*, std::string>> params, FuncDefNode* node) :
        Symbol(ident), _return_type(return_type), _params(params), _node(node) {}
    Type* return_type() { return _return_type; }
    const std::vector<std::tuple<Type*, std::string>>& params() { return _params; }
    FuncDefNode* node() { return _node; }
    std::shared_ptr<Function>& function() { return _function; }
};

class SymbolTable {
protected:
    std::vector<std::string> _symbol_order;
    std::map<std::string, std::shared_ptr<Symbol>> _symbols;
    static uint32_t _block_counter;
    const uint32_t _blockn;
    SymbolTable() : _symbol_order({}), _symbols({}), _blockn(_block_counter) {
        _block_counter++;
    }
protected:
    bool add(std::shared_ptr<Symbol> symbol);
    std::shared_ptr<Symbol> get(const std::string& ident);
public:
    uint32_t blockn() { return _blockn; }
    virtual bool addVar(std::shared_ptr<Var> var) = 0;
    virtual bool addFunc(std::shared_ptr<Func> func) = 0;
    virtual std::shared_ptr<Var> getVar(const std::string& ident) = 0;
    virtual std::shared_ptr<Func> getFunc(const std::string& ident) = 0;
    std::vector<std::shared_ptr<Var>> getVars();
    std::vector<std::shared_ptr<Func>> getFuncs();
};

class GlobalSymbolTable : public SymbolTable {
private:
    std::shared_ptr<MainNode> _main_node;
public:
    GlobalSymbolTable() : SymbolTable() {}
    virtual bool addVar(std::shared_ptr<Var> var) override ;
    virtual bool addFunc(std::shared_ptr<Func> func) override ;
    virtual std::shared_ptr<Func> getFunc(const std::string& ident) override ;
    virtual std::shared_ptr<Var> getVar(const std::string& ident) override ;
    std::shared_ptr<MainNode>& main_node() { return _main_node; }
};

class BlockSymbolTable : public SymbolTable {
private:
    std::shared_ptr<SymbolTable> _base;
public:
    BlockSymbolTable(std::shared_ptr<SymbolTable> base) : _base(base) {}
    std::shared_ptr<GlobalSymbolTable> getGlobal();
    virtual bool addFunc(std::shared_ptr<Func> func) override ;
    virtual std::shared_ptr<Func> getFunc(const std::string& ident) override ;
    virtual bool addVar(std::shared_ptr<Var> var) override ;
    virtual std::shared_ptr<Var> getVar(const std::string& ident) override ;
};

}

}

#endif