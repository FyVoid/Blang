#ifndef BLANG_IR_H
#define BLANG_IR_H

#include "type.hpp"
#include <memory>
#include <string>
#include <tuple>
#include <vector>

namespace blang {
namespace entities {

enum InstructType {
    INSTRUCT_DEF, INSTRUCT_GEP, INSTRUCT_ALLOCA, 
    INSTRUCT_ADD, INSTRUCT_SUB, INSTRUCT_MUL, INSTRUCT_DIV, INSTRUCT_MOD,
    INSTRUCT_STORE, INSTRUCT_LOAD,
    INSTRUCT_RET, INSTRUCT_CALL,
    INSTRUCT_AND, INSTRUCT_OR, INSTRUCT_EQ, INSTRUCT_NEQ,
    INSTRUCT_GE, INSTRUCT_GT, INSTRUCT_LE, INSTRUCT_LT,
    INSTRUCT_ICMP, INSTRUCT_BR,
    INSTRUCT_SEXT, INSTRUCT_TRUNC,
};

class Instruct {
private:
    InstructType _type;
protected:
    Instruct(InstructType type) : _type(type) {}
public:
    InstructType typeId() { return _type; }
    // TODO: add comment
    virtual std::shared_ptr<Value> reg() = 0;
    virtual std::string to_string() = 0;
};

class DefInstruct : public Instruct {
private:
    bool _is_const;
    std::shared_ptr<PtrValue> _var;
    std::shared_ptr<Value> _init;
public:
    DefInstruct(bool is_const, std::shared_ptr<PtrValue> var, std::shared_ptr<Value> init) : 
        Instruct(INSTRUCT_DEF), _is_const(is_const), _var(var), _init(init) {}
    virtual std::shared_ptr<Value> reg() { return _var; }
    virtual std::string to_string() {
        auto flag = _is_const ? "constant" : "global";
        return _var->def() + " = " + flag + " " + _var->getType()->to_string() + " " + _init->ident(); 
    }
};

class GEPInstruct : public Instruct {
private:
    std::shared_ptr<PtrValue> _result;
    std::shared_ptr<PtrValue> _ptr;
    std::shared_ptr<IntConstValue> _elem;
    std::shared_ptr<IntConstValue> _offset;
public:
    GEPInstruct(std::shared_ptr<PtrValue> result, std::shared_ptr<PtrValue> ptr, std::shared_ptr<IntConstValue> elem, std::shared_ptr<IntConstValue> offset) :
        Instruct(INSTRUCT_GEP), _result(result), _ptr(ptr), _elem(elem), _offset(offset) {}
    virtual std::shared_ptr<Value> reg() { return _result; }
    virtual std::string to_string() {
        auto ret = _result->ident() + " = getelementptr ";
        ret += _ptr->getType()->to_string() + ", " + _ptr->to_string();
        if (_elem) {
            ret += ", " + _elem->to_string();
        }
        ret += ", " + _offset->to_string();
        return ret;
    }
};

class AllocaInstruct : public Instruct {
private:
    std::shared_ptr<PtrValue> _var;
public:
    AllocaInstruct(std::shared_ptr<PtrValue> var) :
        Instruct(INSTRUCT_ALLOCA), _var(var) {}
    virtual std::shared_ptr<Value> reg() { return _var; }
    virtual std::string to_string() {
        return _var->ident() + " = alloca " + _var->getType()->to_string(); 
    }
};

class StoreInstruct : public Instruct {
private:
    std::shared_ptr<Value> _from;
    std::shared_ptr<PtrValue> _to;
public:
    StoreInstruct(std::shared_ptr<Value> from, std::shared_ptr<PtrValue> to) :
        Instruct(INSTRUCT_STORE), _from(from), _to(to) {}
    virtual std::shared_ptr<Value> reg() { return _to; }
    virtual std::string to_string() {
        return "store " + _from->getType()->to_string() + " " + _from->ident() + ", " + _to->to_string();
    }
};

class LoadInstruct : public Instruct {
private:
    std::shared_ptr<Value> _from;
    std::shared_ptr<Value> _to;
public:
    LoadInstruct(std::shared_ptr<Value> from, std::shared_ptr<Value> to) :
        Instruct(INSTRUCT_LOAD), _from(from), _to(to) {}
    virtual std::shared_ptr<Value> reg() { return _to; }
    virtual std::string to_string() {
        return _to->ident() + " = load " + _to->getType()->to_string() + ", " + _from->to_string();
    }
};

class RetInstruct : public Instruct {
private:
    std::shared_ptr<Value> _ret_value;
public:
    RetInstruct(std::shared_ptr<Value> ret_value) :
        Instruct(INSTRUCT_RET), _ret_value(ret_value) {}
    virtual std::shared_ptr<Value> reg() { return _ret_value; }
    virtual std::string to_string() {
        std::string ret = "ret";
        if (_ret_value) {
            ret += " " + _ret_value->to_string();
        } else {
            ret += " void";
        }
        return ret;
    }
};

class ArithInstruct : public Instruct {
protected:
    std::shared_ptr<Value> _reg;
    std::shared_ptr<Value> _left;
    std::shared_ptr<Value> _right;
    ArithInstruct(InstructType type, std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        Instruct(type), _reg(reg), _left(left), _right(right) {}
    virtual std::shared_ptr<Value> reg() { return _reg; }
};

class AddInstruct : public ArithInstruct {
public:
    AddInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_ADD, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = add " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class SubInstruct : public ArithInstruct {
public:
    SubInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_SUB, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = sub " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class MulInstruct : public ArithInstruct {
public:
    MulInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_MUL, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = mul " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class DivInstruct : public ArithInstruct {
public:
    DivInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_DIV, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = sdiv " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class ModInstruct : public ArithInstruct {
public:
    ModInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_MOD, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = srem " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class AndInstruct : public ArithInstruct {
public:
    AndInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_AND, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = and " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class OrInstruct : public ArithInstruct {
public:
    OrInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_OR, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = or " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class EqInstruct : public ArithInstruct {
public:
    EqInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_EQ, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = icmp eq " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class NeqInstruct : public ArithInstruct {
public:
    NeqInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_NEQ, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = icmp ne " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class GeInstruct : public ArithInstruct {
public:
    GeInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_GE, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = icmp sge " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class GtInstruct : public ArithInstruct {
public:
    GtInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_GT, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = icmp sgt " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class LeInstruct : public ArithInstruct {
public:
    LeInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_LE, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = icmp sle " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class LtInstruct : public ArithInstruct {
public:
    LtInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) :
        ArithInstruct(INSTRUCT_LT, reg, left, right) {}
    virtual std::string to_string() {
        return _reg->ident() + " = icmp slt " + _left->getType()->to_string() + " " + _left->ident() + ", " + _right->ident();
    }
};

class BrInstruct : public Instruct {
private:
    std::string _label;
public:
    BrInstruct(std::string label) : Instruct(INSTRUCT_BR), _label(label) {}
    virtual std::shared_ptr<Value> reg() { return nullptr; }
    virtual std::string to_string() {
        return "br label %" + _label;
    }
    std::string label() { return _label; }
};

class CondBrInstruct : public Instruct {
private:
    std::shared_ptr<Value> _cond;
    std::string _true_label;
    std::string _false_label;
public:
    CondBrInstruct(std::shared_ptr<Value> cond, std::string true_label, std::string false_label) :
        Instruct(INSTRUCT_BR), _cond(cond), _true_label(true_label), _false_label(false_label) {}
    virtual std::shared_ptr<Value> reg() { return _cond; }
    virtual std::string to_string() {
        return "br i1 " + _cond->ident() + ", label %" + _true_label + ", label %" + _false_label;
    }
    std::shared_ptr<Value> cond() { return _cond; }
    std::string true_label() { return _true_label; }
    std::string false_label() { return _false_label; }
};

class SextInstruct : public Instruct {
private:
    std::shared_ptr<Value> _result;
    std::shared_ptr<Value> _operand;
public:
    SextInstruct(std::shared_ptr<Value> result, std::shared_ptr<Value> operand) :
        Instruct(INSTRUCT_SEXT), _result(result), _operand(operand) {}
    virtual std::shared_ptr<Value> reg() { return _result; }
    virtual std::string to_string() {
        return _result->ident() + " = sext " + _operand->to_string() + " to " + _result->getType()->to_string();
    }
};

class TruncInstruct : public Instruct {
private:
    std::shared_ptr<Value> _result;
    std::shared_ptr<Value> _operand;
public:
    TruncInstruct(std::shared_ptr<Value> result, std::shared_ptr<Value> operand) :
        Instruct(INSTRUCT_TRUNC), _result(result), _operand(operand) {}
    virtual std::shared_ptr<Value> reg() { return _result; }
    virtual std::string to_string() {
        return _result->ident() + " = trunc " + _operand->to_string() + " to " + _result->getType()->to_string();
    }
};

class Block {
private:
    std::string _label;
    std::vector<std::shared_ptr<Instruct>> _instructions;
    std::vector<std::string> _next;
    bool _ended;
public:
    Block(std::string label) : _label(label) {}
    std::string label() { return _label; }
    std::vector<std::shared_ptr<Instruct>>& instructions() { return _instructions; }
    void push_back(std::shared_ptr<Instruct> instruct) { 
        if (!_ended) {
            _instructions.push_back(instruct); 
        }
    }
    std::shared_ptr<Instruct>& last() { return *(_instructions.end() - 1); }
    std::string to_string();
    std::vector<std::string>& next() { return _next; }
    bool& ended() { return _ended; }
};

class Function {
private:
    Type* _ret_type;
    std::string _ident;
    std::vector<std::tuple<Type*, std::string>> _params;
    std::vector<std::shared_ptr<Block>> _blocks;
    std::shared_ptr<Block> _current_block;
    uint64_t _reg_iter;
public:
    Function(Type* ret_type, std::string ident, std::vector<std::tuple<Type*, std::string>> params) :
        _ret_type(ret_type), _ident(ident), _params(params), _blocks({}) {}
    Type* ret_type() { return _ret_type; }
    std::string ident() { return _ident; }
    std::vector<std::tuple<Type*, std::string>> params() { return _params; }
    std::vector<std::shared_ptr<Block>>& blocks() { return _blocks; }
    std::shared_ptr<Block>& current_block() { return _current_block; }
    void setBlock(std::shared_ptr<Block> block) { _current_block = block; }
    void addBlock(std::string label="");
    std::string to_string();
    std::string next_reg() { return std::to_string(_reg_iter++); }
};

class CallInstruct : public Instruct {
private:
    std::shared_ptr<Value> _result;
    std::shared_ptr<Function> _function;
    std::vector<std::shared_ptr<Value>> _params;
public:
    CallInstruct(std::shared_ptr<Value> result, std::shared_ptr<Function> function, std::vector<std::shared_ptr<Value>> params) :
        Instruct(INSTRUCT_CALL), _result(result), _function(function), _params(params) {}
    virtual std::shared_ptr<Value> reg() { return _result; }
    virtual std::string to_string() {
        std::string ret = "";
        if (_result) {
            ret += _result->ident() + " = ";
            ret += "call " + _result->getType()->to_string() + " ";
        } else {
            ret += "call void ";
        }
        ret += "@" + _function->ident() + "(";
        if (!_params.empty()) {
            auto iter = _params.begin();
            while (iter + 1 != _params.end()) {
                ret += (*iter)->to_string() + ", ";
                iter++;
            }
            if (iter != _params.end()) {
                ret += (*iter)->to_string();
        }
        }
        ret += ")";
        return ret;
    }
};

class CallExternalInstruct : public Instruct {
private:
    std::shared_ptr<Value> _result;
    std::string _function;
    std::vector<std::shared_ptr<Value>> _params;
public:
    CallExternalInstruct(std::shared_ptr<Value> result, std::string function, std::vector<std::shared_ptr<Value>> params) :
        Instruct(INSTRUCT_CALL), _result(result), _function(function), _params(params) {}
    virtual std::shared_ptr<Value> reg() { return _result; }
    virtual std::string to_string() {
        std::string ret = "";
        if (_result) {
            ret += _result->ident() + " = ";
            ret += "call " + _result->getType()->to_string() + " ";
        } else {
            ret += "call void ";
        }
        ret += "@" + _function + "(";
        if (!_params.empty()) {
            auto iter = _params.begin();
            while (iter + 1 != _params.end()) {
                ret += (*iter)->to_string() + ", ";
                iter++;
            }
            ret += (*iter)->to_string();
        }
        ret += ")";
        return ret;
    }  
};

class IrModule { 
private:
    std::vector<std::shared_ptr<Instruct>> _global;
    std::map<std::string, std::shared_ptr<Function>> _functions;
    std::shared_ptr<Function> _current_function;
public:
    IrModule() : _global({}) {}
    std::vector<std::shared_ptr<Instruct>>& global() { return _global; }
    std::map<std::string, std::shared_ptr<Function>>& functions() { return _functions; }
    std::shared_ptr<Function> current_function() { return _current_function; }
    std::shared_ptr<Block> current_block() { 
        if (!_current_function) {
            return nullptr;
        }
        return _current_function->current_block(); 
    }
    void setFunction(std::shared_ptr<Function> function) { _current_function = function; }
    std::string to_string();
};

class IrFactory {
private:
    std::shared_ptr<IrModule> _module;
    uint64_t _block_iter;
public:
    IrFactory(std::shared_ptr<IrModule> module) : _module(module) {}
    inline std::string next_reg() { return _module->current_function()->next_reg(); }
    inline std::string next_block() { return std::to_string(_block_iter++); }
    void addFunction(Type* ret_type, std::string ident, std::vector<std::tuple<Type*, std::string>> params);
    void addDefInstruct(bool is_const, std::shared_ptr<PtrValue> var, std::shared_ptr<Value> init);
    void addAllocaInstruct(std::shared_ptr<PtrValue> var);
    void addLoadInstruct(std::shared_ptr<Value> from, std::shared_ptr<Value> to);
    void addStoreInstruct(std::shared_ptr<Value> from, std::shared_ptr<PtrValue> to);
    void addAddInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addSubInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addMulInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addDivInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addModInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addAndInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addOrInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addEqInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addNeqInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addGeInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addGtInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addLtInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addLeInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right);
    void addRetInstruct(std::shared_ptr<Value> ret_value);
    void addCallInstruct(std::shared_ptr<Value> result, std::shared_ptr<Function> function, std::vector<std::shared_ptr<Value>> params);
    void addGepInstruct(std::shared_ptr<PtrValue> result, std::shared_ptr<PtrValue> ptr, std::shared_ptr<IntConstValue> elem, std::shared_ptr<IntConstValue> offset);
    void addBrInstruct(std::string label);
    void addCondBrInstruct(std::shared_ptr<Value> cond, std::string true_label, std::string false_label);
    void addSextInstruct(std::shared_ptr<Value> result, std::shared_ptr<Value> operand);
    void addTruncInstruct(std::shared_ptr<Value> result, std::shared_ptr<Value> operand);
    void addCallExternalInstruct(std::shared_ptr<Value> result, std::string function, std::vector<std::shared_ptr<Value>> params);
};

}
}

#endif