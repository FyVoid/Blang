/**
 * @file symbol_table.hpp
 * @author fyvoid (fyvo1d@outlook.com)
 * @brief Symbol table support for blang
 * @version 1.0
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

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

/**
 * @brief Symbol stored in symbol table
 * Abstract class
 * 
 */
class Symbol {
protected:
    const std::string _ident;
    Symbol(const std::string& ident) : _ident(ident) {}
    virtual ~Symbol() {}
public:
    std::string ident() { return _ident; }
};

/**
 * @brief Variable, sub class of Symbol
 * 
 */
class Var : public Symbol {
private:
    Type* const _type;
    const bool _is_const;
    std::shared_ptr<PtrValue> _value;
    void* _content;
    Var(Type* type, const std::string& ident, bool is_const) :
        Symbol(ident), _type(type), _is_const(is_const) {}
    /**
     * @brief Construct a new single var
     * 
     * @tparam T type of value stored in content
     * @param type Type of the variable
     * @param ident 
     * @param is_const 
     * @param val initilize value
     * @return std::shared_ptr<Var> 
     */
    template<typename T>
    static std::shared_ptr<Var> getSingle(Type* type, const std::string& ident, bool is_const, T val) {
        auto ret = std::shared_ptr<Var>(new Var(type, ident, is_const));
        ret->_content = (void*) new T(val);
        return ret;
    }
    /**
     * @brief Construct a new Array var
     * 
     * @tparam T type of value stored in content
     * @param type Type of the variable
     * @param ident 
     * @param is_const 
     * @param val initilize value
     * @return std::shared_ptr<Var> 
     */
    template<typename T>
    static std::shared_ptr<Var> getArray(Type* type, const std::string& ident, bool is_const, uint32_t length) {
        auto ret = std::shared_ptr<Var>(new Var(ArrayType::get(type, length), ident, is_const));
        ret->_content = (void*) new T[length];
        memset(ret->_content, 0, sizeof(T) * length);
        return ret;
    }
    /**
     * @brief Construct a new pointer var
     * 
     * @tparam T type of value stored in content
     * @param type Type of the pointed target
     * @param ident 
     * @param is_const 
     * @param val initilize value
     * @return std::shared_ptr<Var> 
     */
    template<typename T>
    static std::shared_ptr<Var> getPtr(Type* type, const std::string& ident, bool is_const) {
        auto ret = std::shared_ptr<Var>(new Var(PtrType::get(type), ident, is_const));
        ret->_content = (void*) new T*(0);
        return ret;
    }
public:
    ~Var();
    /**
     * @brief Get a Int Var
     * 
     * @param ident 
     * @param is_const 
     * @param val 
     * @return std::shared_ptr<Var> 
     */
    static std::shared_ptr<Var> getInt(const std::string& ident, bool is_const, int32_t val=0) {
        return getSingle<int32_t>(IntType::get(), ident, is_const, val);
    }
    /**
     * @brief Get a Char Var
     * 
     * @param ident 
     * @param is_const 
     * @param val 
     * @return std::shared_ptr<Var> 
     */
    static std::shared_ptr<Var> getChar(const std::string& ident, bool is_const, char val=0) {
        return getSingle<char>(CharType::get(), ident, is_const, val);
    }
    /**
     * @brief Get a Int Array
     * 
     * @param ident 
     * @param is_const 
     * @param val 
     * @return std::shared_ptr<Var> 
     */
    static std::shared_ptr<Var> getIntArray(const std::string& ident, bool is_const, uint32_t length) {
        return getArray<int32_t>(IntType::get(), ident, is_const, length);
    }
    /**
     * @brief Get a Char Array
     * 
     * @param ident 
     * @param is_const 
     * @param val 
     * @return std::shared_ptr<Var> 
     */
    static std::shared_ptr<Var> getCharArray(const std::string& ident, bool is_const, uint32_t length) {
        return getArray<char>(CharType::get(), ident, is_const, length);
    }
    /**
     * @brief Get a Int Pointer
     * 
     * @param ident 
     * @param is_const 
     * @param val 
     * @return std::shared_ptr<Var> 
     */
    static std::shared_ptr<Var> getIntPtr(const std::string& ident, bool is_const) {
        return getPtr<int32_t>(IntType::get(), ident, is_const);
    }
    /**
     * @brief Get a Char Pointer
     * 
     * @param ident 
     * @param is_const 
     * @param val 
     * @return std::shared_ptr<Var> 
     */
    static std::shared_ptr<Var> getCharPtr(const std::string& ident, bool is_const) {
        return getPtr<char>(CharType::get(), ident, is_const);
    }

    Type* type() { return _type; }
    bool is_const() { return _is_const; }
    /**
     * @brief Try to get content, converted to type T
     * 
     * @tparam T 
     * @return T* 
     */
    template<typename T>
    T* get() { return static_cast<T*>(_content); }
    std::shared_ptr<PtrValue>& value() { return _value; }
};

/**
 * @brief Function, sub class of Symbol
 * 
 */
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

/**
 * @brief SymbolTable base class, provide methods to store and acquired symbols
 * 
 */
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
    /**
     * @brief Add a variable to symbol table
     * 
     * @param var 
     * @return true 
     * @return false 
     */
    virtual bool addVar(std::shared_ptr<Var> var) = 0;
    /**
     * @brief Add a function to symbol table
     * 
     * @param func 
     * @return true 
     * @return false 
     */
    virtual bool addFunc(std::shared_ptr<Func> func) = 0;
    /**
     * @brief Get a variable from symbol table
     * 
     * @param ident 
     * @return std::shared_ptr<Var> 
     */
    virtual std::shared_ptr<Var> getVar(const std::string& ident) = 0;
    /**
     * @brief Get a function from symbol table
     * 
     * @param ident 
     * @return std::shared_ptr<Func> 
     */
    virtual std::shared_ptr<Func> getFunc(const std::string& ident) = 0;
    std::vector<std::shared_ptr<Var>> getVars();
    std::vector<std::shared_ptr<Func>> getFuncs();
};

/**
 * @brief Global symbol table
 * 
 */
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

/**
 * @brief Symbol table of certain block
 * 
 */
class BlockSymbolTable : public SymbolTable {
private:
    /**
     * @brief Symbol table of block containing this block
     * Outest is global symbol table
     * 
     */
    std::shared_ptr<SymbolTable> _base;
public:
    BlockSymbolTable(std::shared_ptr<SymbolTable> base) : _base(base) {}
    std::shared_ptr<GlobalSymbolTable> getGlobal();
    /**
     * @brief Add a function to global symbol table
     * 
     * @param func 
     * @return true 
     * @return false 
     */
    virtual bool addFunc(std::shared_ptr<Func> func) override ;
    /**
     * @brief Get function from global symbol table
     * 
     * @param ident 
     * @return std::shared_ptr<Func> 
     */
    virtual std::shared_ptr<Func> getFunc(const std::string& ident) override ;
    virtual bool addVar(std::shared_ptr<Var> var) override ;
    /**
     * @brief Try to search for variable from current table
     * Recursively search outer tables, until global symbol table
     * 
     * @param ident 
     * @return std::shared_ptr<Var> 
     */
    virtual std::shared_ptr<Var> getVar(const std::string& ident) override ;
};

}

}

#endif