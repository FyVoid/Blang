/**
 * @file evaluator.hpp
 * @author fyvoid (fyvo1d@outlook.com)
 * @brief Evaluator support for ast
 * @version 1.0
 * @date 2024-11-25
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef BLANG_EVALUATOR_H
#define BLANG_EVALUATOR_H

#include "ast.hpp"
#include "symbol_table.hpp"
#include "type.hpp"
#include "visitor.hpp"
#include <cstdint>
#include <memory>
#include <stdexcept>

namespace blang {

namespace tools {

using namespace entities;

/**
 * @brief A Visitor sub class, evaluate constant expression in ast
 * 
 */
class Evaluator : public Visitor {
private:
    std::shared_ptr<SymbolTable> _current_table;
    int32_t _value;
    /**
     * @brief Calculate binary exp according to op
     * 
     * @param left 
     * @param right 
     * @param op 
     * @return int32_t 
     */
    int32_t calBinary(int32_t left, int32_t right, Op op) {
        switch (op) {
            case OP_ADD:    return left +  right;
            case OP_MINUS:  return left -  right;
            case OP_MUL:    return left *  right;
            case OP_DIV:    return left /  right;
            case OP_MOD:    return left %  right;
            case OP_AND:    return left && right;
            case OP_OR:     return left || right;
            case OP_EQ:     return left == right;
            case OP_NEQ:    return left != right;
            case OP_GE:     return left >= right;
            case OP_GT:     return left >  right;
            case OP_LE:     return left <= right;
            case OP_LT:     return left <  right;
            default:
                throw std::runtime_error("invalid op");
        }

        return 0;
    }
    virtual void visit(BinaryExpNode& node) override {
        node.left()->accept(*this);
        auto left = _value;
        node.right()->accept(*this);
        auto right = _value;
        _value = calBinary(left, right, node.op());
    }
    virtual void visit(UnaryExpNode& node) override {
        if (node.primary_exp()) {
            node.primary_exp()->accept(*this);
        } else if (node.unary_exp()) {
            node.unary_exp()->accept(*this);
            _value = node.op() == OP_ADD ? _value 
                    : node.op() == OP_MINUS ? -_value 
                    : node.op() == OP_NOT ? !_value : 0;
        } else if (!node.ident().empty()) {
            throw std::runtime_error("cannot evaluate function call");
        }
    }
    virtual void visit(PrimaryExpNode& node) override {
        if (node.value()) {
            node.value()->accept(*this);
        } else if (node.lval()) {
            node.lval()->accept(*this);
        } else if (node.exp()) {
            node.exp()->accept(*this);
        }
    }
    virtual void visit(ValueNode& node) override {
        if (Type::is_same(node.type(), IntType::get())) {
            _value = node.get<int32_t>();
        } else if (Type::is_same(node.type(), CharType::get())) {
            _value = static_cast<int32_t>(node.get<char>());
        }
    }
    virtual void visit(LValNode& node) override {
        auto var = _current_table->getVar(node.ident());
        if (!var) {
            throw std::runtime_error("var not found");
        }
        if (!var->is_const()) {
            throw std::runtime_error("cannot evaluate non const");
        }
        if (Type::is_same(var->type(), IntType::get())) {
            _value = *var->get<int32_t>();
        } else if (Type::is_same(var->type(), CharType::get())) {
            _value = static_cast<int32_t>(*var->get<char>());
        } else if (auto array_t = dynamic_cast<ArrayType*>(var->type()); array_t) {
            auto content_t = array_t->type();
            auto index = evaluate(*node.exp(), _current_table);
            if (Type::is_same(content_t, IntType::get())) {
                _value = *(var->get<int32_t>() + index);
            } else if (Type::is_same(content_t, CharType::get())) {
                _value = static_cast<char>(*(var->get<char>() + index));
            }
        }
    }
public:
    Evaluator();
    /**
     * @brief Evaluate a constatnt expression in ast
     * throw std::runtime_error if expression cannot be evaluated
     * 
     * @param node Exp to be evaluated
     * @param current_table Current symbol table
     * @return int32_t 
     */
    int32_t evaluate(ExpNode& node, std::shared_ptr<SymbolTable> current_table) {
        _current_table = current_table;
        _value = 0;
        node.accept(*this);
        return _value;
    }
};

}

}

#endif