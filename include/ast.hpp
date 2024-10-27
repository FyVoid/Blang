#ifndef BLANG_AST_H
#define BLANG_AST_H

#include <cstdint>
#include <memory>
#include <vector>
#include <variant>
#include "type.hpp"

namespace blang {

namespace entities {

enum Op {
    OP_ADD, OP_MINUS, OP_NOT,
    OP_MUL, OP_DIV, OP_MOD,
    OP_LT, OP_GT, OP_LE, OP_GE,
    OP_EQ, OP_NEQ,
    OP_AND, OP_OR,
    OP_EMPTY,
};

class Visitor;

class AstNode {
protected:
    uint32_t _line;
    AstNode(uint32_t line) : _line(line) {}
public:
    uint32_t line() { return _line; }
    virtual void accept(Visitor& visitor) = 0;
};

class ExpNode : public AstNode {
private:
    const Op _op;
protected:
    ExpNode(uint32_t line, Op op) : AstNode(line), _op(op) {}
public:
    Op op() { return _op; }
};

class BinaryExpNode : public ExpNode {
private:
    std::shared_ptr<ExpNode> _left;
    std::shared_ptr<ExpNode> _right;
public:
    BinaryExpNode(uint32_t line, Op op, std::shared_ptr<ExpNode> left, std::shared_ptr<ExpNode> right) :
        ExpNode(line, op), _left(left), _right(right) {}
    std::shared_ptr<ExpNode> left() { return _left; }
    std::shared_ptr<ExpNode> right() { return _right; }
    void accept(Visitor& visitor) override;
};

class PrimaryExpNode;
class FuncRParamsNode;

class UnaryExpNode : public ExpNode {
private:
    std::shared_ptr<UnaryExpNode> _unary_exp;
    std::shared_ptr<PrimaryExpNode> _primary_exp;
    const std::string _ident;
    std::shared_ptr<FuncRParamsNode> _func_rparams;
public:
    UnaryExpNode(uint32_t line, Op op, std::shared_ptr<UnaryExpNode> unary_exp) :
        ExpNode(line, op), _unary_exp(unary_exp), _primary_exp(nullptr), _ident(""), _func_rparams(nullptr) {}
    UnaryExpNode(uint32_t line, std::shared_ptr<PrimaryExpNode> primary_exp) :
        ExpNode(line, OP_EMPTY), _unary_exp(nullptr), _primary_exp(primary_exp), _ident(""), _func_rparams(nullptr) {}
    UnaryExpNode(uint32_t line, const std::string& ident, std::shared_ptr<FuncRParamsNode> func_rparams) :
        ExpNode(line, OP_EMPTY), _unary_exp(nullptr), _primary_exp(nullptr), _ident(ident), _func_rparams(func_rparams) {}
    std::shared_ptr<UnaryExpNode> unary_exp() { return _unary_exp; }
    std::shared_ptr<PrimaryExpNode> primary_exp() { return _primary_exp; }
    const std::string& ident() { return _ident; }
    std::shared_ptr<FuncRParamsNode> func_rparams() { return _func_rparams; }
    void accept(Visitor& visitor) override;
};

enum InitType {
    INIT_SINGLE, INIT_ARRAY, INIT_STRING,
};

class InitValNode : public AstNode {
private:
    InitType _type;
    std::vector<std::shared_ptr<ExpNode>> _exps;
    std::string _str;
    const bool _is_const;
    InitValNode(uint32_t line, InitType type, std::vector<std::shared_ptr<ExpNode>> vec, const std::string& str, bool is_const) :
        AstNode(line), _type(type), _exps(vec), _str(str), _is_const(is_const) {}
public:
    // single Exp
    InitValNode(uint32_t line, std::shared_ptr<ExpNode> exp, bool is_const) :
        InitValNode(line, INIT_SINGLE, std::vector<std::shared_ptr<ExpNode>>{exp}, "", is_const) {}
    // { Exp, ... }
    InitValNode(uint32_t line, std::vector<std::shared_ptr<ExpNode>>& exps, bool is_const) :
        InitValNode(line, INIT_ARRAY, exps, "", is_const) {}
    // String
    InitValNode(uint32_t line, const std::string& str, bool is_const) :
        InitValNode(line, INIT_STRING, std::vector<std::shared_ptr<ExpNode>>(), str, is_const) {}
    void accept(Visitor& visitor) override;
    const InitType& type() { return _type; }
    const std::string& str() { return _str; }
    bool is_const() { return _is_const; }
    std::shared_ptr<ExpNode> exp() {
        if (_type == INIT_SINGLE) return _exps[0];
        return std::shared_ptr<ExpNode>();
    }
    std::vector<std::shared_ptr<ExpNode>> exps() {
        return _exps;
    }
};

class DefNode : public AstNode {
private:
    const std::string _ident;
    const bool _is_const;
    std::shared_ptr<InitValNode> _init_val;
    std::shared_ptr<ExpNode> _array_exp;
public:
    DefNode(uint32_t line, const std::string& ident, std::shared_ptr<InitValNode> exp, bool is_const,
        std::shared_ptr<ExpNode> array_exp=std::shared_ptr<ExpNode>()) :
        AstNode(line), _ident(ident), _init_val(exp), _array_exp(array_exp), _is_const(is_const) {}
    void accept(Visitor& visitor) override;
    const std::string& ident() { return _ident; }
    bool is_const() { return _is_const; }
    std::shared_ptr<InitValNode> init_val() { return _init_val; }
    std::shared_ptr<ExpNode> array_exp() { return _array_exp; }
};

class DeclNode : public AstNode {
private:
    const Type* _type;
    const bool _is_const;
    std::vector<std::shared_ptr<DefNode>> _nodes;
public:
    DeclNode(uint32_t line, Type* type, bool is_const, std::vector<std::shared_ptr<DefNode>> vec) :
        AstNode(line), _type(type), _is_const(is_const), _nodes(vec) {}
    void accept(Visitor& visitor) override;
    const Type* type() { return _type; }
    bool is_const() { return _is_const; }
    std::vector<std::shared_ptr<DefNode>>& defs() { return _nodes; }
};

class FuncRParamsNode : public AstNode {
private:
    std::vector<std::shared_ptr<ExpNode>> _nodes;
public:
    FuncRParamsNode(uint32_t line, std::vector<std::shared_ptr<ExpNode>> vec) :
        AstNode(line), _nodes(vec) {}
    std::vector<std::shared_ptr<ExpNode>> nodes() { return _nodes; }
    void accept(Visitor& visitor) override;
};

class ValueNode : public AstNode {
private:
    const Type* _type;
    const std::variant<int32_t, char> _value;
public:
    ValueNode(uint32_t line, int32_t value) : AstNode(line), _type(IntType::get()), _value(value) {}
    ValueNode(uint32_t line, char value) : AstNode(line), _type(CharType::get()), _value(value) {}
    const Type* type() { return _type; }
    void accept(Visitor& visitor) override;
    template<typename T>
    T get() { return std::get<T>(_value); } // may throw error
};

enum RValType {
    RVAL_EXP, RVAL_GETINT, RVAL_GETCHAR,
};

class RValNode : public AstNode {
private:
    const RValType _type;
    std::shared_ptr<ExpNode> _exp;
public:
    RValNode(uint32_t line, RValType type) :
        AstNode(line), _type(type), _exp(nullptr) {}
    RValNode(uint32_t line, std::shared_ptr<ExpNode> exp) :
        AstNode(line), _type(RVAL_EXP), _exp(exp) {}
    RValType type() { return _type; }
    std::shared_ptr<ExpNode> exp() { return _exp; }
    void accept(Visitor& visitor) override;
};

class LValNode;

class PrimaryExpNode : public AstNode {
private:
    std::shared_ptr<ExpNode> _exp;
    std::shared_ptr<LValNode> _lval;
    std::shared_ptr<ValueNode> _value;
public:
    PrimaryExpNode(uint32_t line, std::shared_ptr<ExpNode> exp) :
        AstNode(line), _exp(exp), _lval(nullptr), _value(nullptr) {}
    PrimaryExpNode(uint32_t line, std::shared_ptr<LValNode> lval) :
        AstNode(line), _exp(nullptr), _lval(lval), _value(nullptr) {}
    PrimaryExpNode(uint32_t line, std::shared_ptr<ValueNode> value) :
        AstNode(line), _exp(nullptr), _lval(nullptr), _value(value) {}
    std::shared_ptr<ExpNode> exp() {
        return _exp;
    }
    std::shared_ptr<LValNode> lval() {
        return _lval;
    }
    std::shared_ptr<ValueNode> value() {
        return _value;
    }
    void accept(Visitor& visitor) override;
};

class LValNode : public AstNode {
private:
    const std::string _ident;
    std::shared_ptr<ExpNode> _exp;
public:
    LValNode(uint32_t line, const std::string& ident, std::shared_ptr<ExpNode> exp) :
        AstNode(line), _ident(ident), _exp(exp) {}
    const std::string& ident() { return _ident; }
    std::shared_ptr<ExpNode> exp() { return _exp; }
    void accept(Visitor& visitor) override;
};

class BlockNode;

class StmtNode : public AstNode {
protected:
    StmtNode(uint32_t line) : AstNode(line) {}
};

class AssignStmtNode : public StmtNode {
private:
    std::shared_ptr<LValNode> _lval;
    std::shared_ptr<RValNode> _rval;
public:
    AssignStmtNode(uint32_t line, std::shared_ptr<LValNode> lval, std::shared_ptr<RValNode> rval) :
        StmtNode(line), _lval(lval), _rval(rval) {}
    std::shared_ptr<LValNode> lval() { return _lval; }
    std::shared_ptr<RValNode> rval() { return _rval; }
    void accept(Visitor& visitor) override;
};

class PrintfStmtNode : public StmtNode {
private:
    std::string _fmt;
    std::vector<std::shared_ptr<ExpNode>> _exps;
public:
    PrintfStmtNode(uint32_t line, const std::string& fmt, std::vector<std::shared_ptr<ExpNode>> exps=std::vector<std::shared_ptr<ExpNode>>()) : 
        StmtNode(line), _fmt(fmt), _exps(exps) {}
    const std::string fmt() { return _fmt; }
    std::vector<std::shared_ptr<ExpNode>> exps() { return _exps; }
    void accept(Visitor& visitor) override;
};

class ExpStmtNode : public StmtNode {
private:
    std::shared_ptr<ExpNode> _exp;
public:
    ExpStmtNode(uint32_t line, std::shared_ptr<ExpNode> exp=nullptr) :
        StmtNode(line), _exp(exp) {}
    std::shared_ptr<ExpNode> exp() { return _exp; }
    void accept(Visitor& visitor) override;
};

class BlockStmtNode : public StmtNode {
private:
    std::shared_ptr<BlockNode> _block;
public:
    BlockStmtNode(uint32_t line, std::shared_ptr<BlockNode> block) :
        StmtNode(line), _block(block) {}
    std::shared_ptr<BlockNode> block() { return _block; }
    void accept(Visitor& visitor) override;
};

class IfStmtNode : public StmtNode {
private:
    std::shared_ptr<ExpNode> _cond;
    std::shared_ptr<StmtNode> _if_stmt;
    std::shared_ptr<StmtNode> _else_stmt;
public:
    IfStmtNode(uint32_t line, std::shared_ptr<ExpNode> cond, std::shared_ptr<StmtNode> if_stmt, std::shared_ptr<StmtNode> else_stmt) :
        StmtNode(line), _cond(cond), _if_stmt(if_stmt), _else_stmt(else_stmt) {}
    std::shared_ptr<ExpNode> cond() { return _cond; }
    std::shared_ptr<StmtNode> if_stmt() { return _if_stmt; }
    std::shared_ptr<StmtNode> else_stmt() { return _else_stmt; }
    void accept(Visitor& visitor) override;
};

class ForStmtNode : public StmtNode {
private:
    std::shared_ptr<StmtNode> _for_in;
    std::shared_ptr<ExpNode> _cond;
    std::shared_ptr<StmtNode> _for_out;
    std::shared_ptr<StmtNode> _stmt;
public:
    ForStmtNode(uint32_t line, std::shared_ptr<StmtNode> for_in, std::shared_ptr<ExpNode> cond, std::shared_ptr<StmtNode> for_out, std::shared_ptr<StmtNode> stmt) :
        StmtNode(line), _for_in(for_in), _cond(cond), _for_out(for_out), _stmt(stmt) {}
    std::shared_ptr<StmtNode> for_in() { return _for_in; }
    std::shared_ptr<ExpNode> cond() { return _cond; }
    std::shared_ptr<StmtNode> for_out() { return _for_out; }
    std::shared_ptr<StmtNode> stmt() { return _stmt; }
    void accept(Visitor& visitor) override;
};

class BreakStmtNode : public StmtNode {
public:
    BreakStmtNode(uint32_t line) : StmtNode(line) {}
    void accept(Visitor& visitor) override;
};

class ContinueStmtNode : public StmtNode {
public:
    ContinueStmtNode(uint32_t line) : StmtNode(line) {}
    void accept(Visitor& visitor) override;
};

class ReturnStmtNode : public StmtNode {
private:
    std::shared_ptr<ExpNode> _exp;
public:
    ReturnStmtNode(uint32_t line, std::shared_ptr<ExpNode> exp=nullptr) :
        StmtNode(line), _exp(exp) {}
    std::shared_ptr<ExpNode> exp() { return _exp; }
    void accept(Visitor& visitor) override;
};

class BlockItemNode: public AstNode {
private:
    std::shared_ptr<AstNode> _item;
public:
    BlockItemNode(uint32_t line, std::shared_ptr<DeclNode> decl) :
        AstNode(line), _item(decl) {}
    BlockItemNode(uint32_t line, std::shared_ptr<StmtNode> stmt) : 
        AstNode(line), _item(stmt) {}
    void accept(Visitor& visitor) override;
    std::shared_ptr<DeclNode> decl() {
        return std::dynamic_pointer_cast<DeclNode>(_item);
    }
    std::shared_ptr<StmtNode> stmt() {
        return std::dynamic_pointer_cast<StmtNode>(_item);
    }
};

class BlockNode : public AstNode {
private:
    std::vector<std::shared_ptr<BlockItemNode>> _items;
public:
    BlockNode(uint32_t line, std::vector<std::shared_ptr<BlockItemNode>> vec) :
        AstNode(line), _items(vec) {}
    void accept(Visitor& visitor) override;
    std::vector<std::shared_ptr<BlockItemNode>> items() { return _items; }
};

class FuncFParamsNode : public AstNode {
private:
    std::vector<std::tuple<Type*, std::string>> _params;
public:
    FuncFParamsNode(uint32_t line, std::vector<std::tuple<Type*, std::string>> vec
        = std::vector<std::tuple<Type*, std::string>>()) : 
        AstNode(line), _params(vec) {}
    void accept(Visitor& visitor) override;
    std::vector<std::tuple<Type*, std::string>>& params() { return _params; }
};

class FuncDefNode : public AstNode {
private:
    const Type* _type;
    const std::string _ident;
    std::shared_ptr<FuncFParamsNode> _params;
    std::shared_ptr<BlockNode> _block;
public:
    FuncDefNode(uint32_t line, Type* type, const std::string& ident, std::shared_ptr<BlockNode> block,
        std::shared_ptr<FuncFParamsNode> params=nullptr) :
        AstNode(line), _type(type), _ident(ident), _params(params), _block(block) {}
    void accept(Visitor& visitor) override;
    const Type*& type() { return _type; }
    const std::string& ident() { return _ident; }
    std::shared_ptr<FuncFParamsNode> params() { return _params; }
    std::shared_ptr<BlockNode> block() { return _block; }
};

class MainNode : public AstNode {
private:
    std::shared_ptr<BlockNode> _block;
public:
    MainNode(uint32_t line, std::shared_ptr<BlockNode> block) :
        AstNode(line), _block(block) {}
    std::shared_ptr<BlockNode> block() { return _block; }
    void accept(Visitor& visitor) override;
};

class CompNode : public AstNode {
private:
    std::shared_ptr<CompNode> _comp;
    std::shared_ptr<AstNode> _unit;
public:
    CompNode(uint32_t line, std::shared_ptr<DeclNode> decl, std::shared_ptr<CompNode> comp=nullptr) :
        AstNode(line), _comp(comp), _unit(decl) {}
    CompNode(uint32_t line, std::shared_ptr<FuncDefNode> func_def, std::shared_ptr<CompNode> comp=nullptr) :
        AstNode(line), _comp(comp), _unit(func_def) {}
    CompNode(uint32_t line, std::shared_ptr<MainNode> main_func_def, std::shared_ptr<CompNode> comp=nullptr) :
        AstNode(line), _comp(comp), _unit(main_func_def) {}
    void accept(Visitor& visitor) override;
    std::shared_ptr<CompNode> comp() { return _comp; }
    std::shared_ptr<DeclNode> decl() { 
        return std::dynamic_pointer_cast<DeclNode>(_unit); 
    };
    std::shared_ptr<FuncDefNode> func_def() {
        return std::dynamic_pointer_cast<FuncDefNode>(_unit);
    }
    std::shared_ptr<MainNode> main_func_def() {
        return std::dynamic_pointer_cast<MainNode>(_unit);
    }
};

}

}

#endif