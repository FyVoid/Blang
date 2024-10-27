#ifndef BLANG_TYPE_H
#define BLANG_TYPE_H

#include <cstdint>
#include <map>
#include <stdexcept>
#include <tuple>
#include <unordered_map>

namespace blang {

namespace entities {

enum BlangType {
    TYPE_INT, TYPE_CHAR, TYPE_PTR, TYPE_VOID,
};

class Type {
protected:
    BlangType _type;
    Type(BlangType type) : _type(type) {}
public:
    BlangType type() { return _type; }
    static bool is_same(Type* t1, Type* t2) {
        return t1 == t2;
    }
};

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
};

class CharType : public ValueType {
protected:
    CharType() : ValueType(TYPE_CHAR) {}
public:
    static CharType* get() {
        static CharType* instance = new CharType();
        return instance;
    }
};

class VoidType : public Type {
protected:
    VoidType() : Type(TYPE_VOID) {}
public:
    static VoidType* get() {
        static VoidType* instance = new VoidType();
        return instance;
    }
};

class PtrType : public Type {
protected:
    Type* _next;
    PtrType(Type* type) : Type(TYPE_PTR), _next(type) {
        if (type->type() == TYPE_VOID) {
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
};

class ArrayType : public Type {
protected:
    uint32_t _length;
    Type* _type;
    ArrayType(Type* type, uint32_t length) : Type(type->type()), _type(type), _length(length) {
        if (type->type() == TYPE_VOID) {
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
    uint32_t length() { return _length; }
};

}

}

#endif