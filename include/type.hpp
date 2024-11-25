/**
 * @file type.hpp
 * @author fyvoid (fyvo1d@outlook.com)
 * @brief Type support for blang
 * @version 1.0
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef BLANG_TYPE_H
#define BLANG_TYPE_H

#include <cstdint>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace blang {

namespace entities {

/**
 * @brief Type id enums
 * 
 */
enum BlangType {
    TYPE_INT, TYPE_CHAR, TYPE_PTR, TYPE_VOID, TYPE_BOOL,
};

/**
 * @brief Type base class
 * 
 */
class Type {
protected:
    BlangType _type_id;
    Type(BlangType type) : _type_id(type) {}
public:
    BlangType type_id() { return _type_id; }
    static bool is_same(Type* t1, Type* t2) {
        return t1 == t2;
    }
    virtual ~Type() = default;
    virtual std::string to_string() = 0;
};

/**
 * @brief Value type base class
 * 
 */
class ValueType : public Type {
protected:
    ValueType(BlangType type) : Type(type) {
        if (type == TYPE_PTR || type == TYPE_VOID) {
            throw std::runtime_error("Value type cannot be ptr or void!");
        }
    }
};

class IntType : public ValueType {
protected:
    IntType() : ValueType(TYPE_INT) {}
public:
    static IntType* get() {
        static IntType* instance = new IntType();
        return instance;
    }
    virtual std::string to_string() { return "i32"; }
};

class CharType : public ValueType {
protected:
    CharType() : ValueType(TYPE_CHAR) {}
public:
    static CharType* get() {
        static CharType* instance = new CharType();
        return instance;
    }
    virtual std::string to_string() { return "i8"; }
};

class BoolType: public ValueType {
protected:
    BoolType() : ValueType(TYPE_BOOL) {}
public:
    static BoolType* get() {
        static BoolType* instance = new BoolType();
        return instance;
    }
    virtual std::string to_string() { return "i1"; }
};

class VoidType : public Type {
protected:
    VoidType() : Type(TYPE_VOID) {}
public:
    static VoidType* get() {
        static VoidType* instance = new VoidType();
        return instance;
    }
    virtual std::string to_string() { return "void"; }
};

class PtrType : public Type {
protected:
    Type* _next;
    PtrType(Type* type) : Type(TYPE_PTR), _next(type) {
        if (type->type_id() == TYPE_VOID) {
            throw std::runtime_error("Ptr type cannot point to type void!");
        }
    }
public:
    static PtrType* get(Type* type) {
        static std::unordered_map<Type*, PtrType*> type_map{};
        if (auto iter = type_map.find(type); iter != type_map.end()) {
            return iter->second;
        } else {
            auto ptr_t = new PtrType(type);
            type_map.insert({type, ptr_t});
            return ptr_t;
        }
    } 
    Type* next() { return _next; }
    virtual std::string to_string() { return _next->to_string() + "*"; }
};

class ArrayType : public Type {
protected:
    uint32_t _length;
    Type* _type;
    ArrayType(Type* type, uint32_t length) : Type(type->type_id()), _type(type), _length(length) {
        if (type->type_id() == TYPE_VOID) {
            throw std::runtime_error("Array type cannot be void!");
        }
    }
public:
    static ArrayType* get(Type* type, uint32_t length) {
        static std::map<std::tuple<Type*, uint32_t>, ArrayType*> type_map{};
        auto key = std::make_tuple(type, length);
        if (auto iter = type_map.find(key); iter != type_map.end()) {
            return iter->second;
        } else {
            auto array_t = new ArrayType(type, length);
            type_map.insert({key, array_t});
            return array_t;
        }
    }
    Type* type() { return _type; }
    uint32_t length() { return _length; }
    virtual std::string to_string() {
        return "[" + std::to_string(_length) + " x " + _type->to_string() + "]";
    }
};

class Value {
protected:
    Type* _type;
    Value(Type* type) : _type(type) {}
public:
    Type* getType() { return _type; }
    virtual std::string to_string() = 0;
    virtual std::string ident() = 0;
};

class IntConstValue : public Value {
private:
    int32_t _content;
public:
    IntConstValue(int32_t content) : Value(IntType::get()), _content(content) {}
    virtual std::string to_string() { return "i32 " + std::to_string(_content); }
    virtual std::string ident() { return std::to_string(_content); }
};

class CharConstValue : public Value {
private:
    char _content;
public:
    CharConstValue(char content) : Value(CharType::get()), _content(content) {}
    virtual std::string to_string() { return "i8 " + std::to_string(static_cast<int32_t>(_content)); }
    virtual std::string ident() { return std::to_string(_content); }    
};

class BoolConstValue : public Value {
private:
    bool _content;
public:
    BoolConstValue(bool content) : Value(BoolType::get()), _content(content) {}
    virtual std::string to_string() { return "i1 " + std::to_string(static_cast<int32_t>(_content)); }
    virtual std::string ident() { return std::to_string(_content); }
};

class ArrayValue : public Value {
private:
    std::vector<std::shared_ptr<Value>> _content;
public:
    ArrayValue(Type* type, std::vector<std::shared_ptr<Value>> content) : Value(type), _content(content) {}
    virtual std::string to_string() {
        std::string ret = getType()->to_string() + " [";
        for (int i = 0; i < _content.size() - 1; i++) {
            ret += _content[i]->to_string() + ", ";
        }
        ret += _content[_content.size() - 1]->to_string() + "]";
        return ret;
    }
    virtual std::string ident() {
        std::string ret = "[";
        for (int i = 0; i < _content.size() - 1; i++) {
            ret += _content[i]->to_string() + ", ";
        }
        ret += _content[_content.size() - 1]->to_string() + "]";
        return ret;
    }
};

class IntValue : public Value {
private:
    std::string _ident;
public:
    IntValue(std::string ident) : Value(IntType::get()), _ident(ident) {}
    virtual std::string to_string() { return "i32 %" + _ident; }
    virtual std::string ident() { return "%" + _ident; }
};

class CharValue : public Value {
private:
    std::string _ident;
public:
    CharValue(std::string ident) : Value(CharType::get()), _ident(ident) {}
    virtual std::string to_string() { return "i8 %" + _ident; }
    virtual std::string ident() { return "%" + _ident; }
};

class BoolValue : public Value {
private:
    std::string _ident;
public:
    BoolValue(std::string ident) : Value(BoolType::get()), _ident(ident) {}
    virtual std::string to_string() { return "i1 %" + _ident; }
    virtual std::string ident() { return "%" + _ident; }
};

class PtrValue : public Value {
private:
    bool _global;
    std::string _ident;
public:
    PtrValue(Type* type, bool global, std::string ident) : Value(type), _global(global), _ident(ident) {}
    std::string flag() { return _global ? "@" : "%"; }
    virtual std::string to_string() { return getType()->to_string() + "* " + flag() + _ident; }
    virtual std::string ident() {
        return flag() + _ident; 
    }
    std::string def() { return flag() + _ident; }
};

}

}

#endif