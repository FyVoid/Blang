#ifndef BLANG_SYNTAX_CHECKER_H
#define BLANG_SYNTAX_CHECKER_H

#include "ast.hpp"
#include "buaa.hpp"
#include "evaluator.hpp"
#include "logger.hpp"
#include "symbol_table.hpp"
#include "type.hpp"
#include "visitor.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace blang {

namespace frontend {

using namespace entities;
using namespace tools;

class SyntaxChecker : public Visitor {
private:
    std::shared_ptr<Logger> _logger;
    std::shared_ptr<GlobalSymbolTable> _global_table;
    std::shared_ptr<SymbolTable> _current_table;
    class Checker : public Visitor {
    protected:
        std::shared_ptr<Logger> _logger;
        std::shared_ptr<SymbolTable> _current_table;
        Checker(std::shared_ptr<Logger> logger, std::shared_ptr<SymbolTable> table=nullptr) :
            _logger(logger), _current_table(table) {}
        static int32_t evaluate(ExpNode& node, std::shared_ptr<SymbolTable> current_table) {
            static Evaluator evaluator{};
            try {
                return evaluator.evaluate(node, current_table);
            } catch (std::runtime_error err) {
                return -1;
            }
        }
    };
    // b
    class DefChecker : public Checker {
    private:
        Type* _type;
        std::vector<std::tuple<Type*, std::string>> _params;
        std::shared_ptr<BlockSymbolTable> _func_block;
        template<typename T>
        std::vector<T> getInitVal(InitValNode& node) {
            std::vector<T> ret{};

            if (node.type() == entities::INIT_SINGLE) {
                ret.push_back(static_cast<T>(evaluate(*node.exp(), _current_table)));
            } else if (node.type() == entities::INIT_ARRAY) {
                for (auto exp : node.exps()) {
                    ret.push_back(static_cast<T>(evaluate(*exp, _current_table)));
                }
            } else if (node.type() == entities::INIT_STRING) {
                for (auto ch : node.str()) {
                    ret.push_back(static_cast<T>(ch));
                }
            }

            return ret;
        }
        virtual void visit(DefNode& node) override {
            Type* base_t = _type;
            std::shared_ptr<Var> var;
            std::string log_type = node.is_const() ? "Const" : "";
            if (node.array_exp()) {
                auto length = evaluate(*node.array_exp(), _current_table);
                if (Type::is_same(base_t, IntType::get())) {
                    var = Var::getIntArray(node.ident(), node.is_const(), length);
                    if (node.is_const()) {
                        auto init_val = getInitVal<int32_t>(*node.init_val());
                        memcpy(var->get<int32_t>(), init_val.data(), sizeof(int32_t) * std::min(length, static_cast<int32_t>(init_val.size())));
                    }
                    log_type += "IntArray";
                } else if (Type::is_same(base_t, CharType::get())) {
                    var = Var::getCharArray(node.ident(), node.is_const(), length);
                    if (node.is_const()) {
                        auto init_val = getInitVal<char>(*node.init_val());
                        memcpy(var->get<char>(), init_val.data(), sizeof(char) * std::min(length, static_cast<int32_t>(init_val.size())));
                    }
                    log_type += "CharArray";
                }
            } else {
                if (Type::is_same(base_t, IntType::get())) {
                    var = Var::getInt(node.ident(), node.is_const());
                    if (node.is_const()) {
                        auto init_val = getInitVal<int32_t>(*node.init_val());
                        memcpy(var->get<int32_t>(), init_val.data(), sizeof(int32_t));
                    }
                    log_type += "Int";
                } else if (Type::is_same(base_t, CharType::get())) {
                    var = Var::getChar(node.ident(), node.is_const());
                    if (node.is_const()) {
                        auto init_val = getInitVal<char>(*node.init_val());
                        memcpy(var->get<char>(), init_val.data(), sizeof(char));
                    }
                    log_type += "Char";
                }
            }
            if (!_current_table->addVar(var)) {
                _logger->logError(std::make_shared<ErrorLog>(node.line(), "def duplicate", buaa::ERROR_IDENT_REDEF));
            } else {
                _logger->log(std::make_shared<SyntaxLog>(node.line(), _current_table->blockn(), var->ident(), log_type));
            }
        }
        virtual void visit(FuncDefNode& node) override {
            Type* type = node.type();
            _func_block = std::make_shared<BlockSymbolTable>(_current_table);
            if (node.params()) {
                node.params()->accept(*this);
                _params = node.params()->params();
            }
            std::shared_ptr<Func> func = std::make_shared<Func>(node.type(), node.ident(), _params, &node);

            std::string log_type;
            if (Type::is_same(node.type(), VoidType::get())) {
                log_type = "VoidFunc";
            } else if (Type::is_same(node.type(), IntType::get())) {
                log_type = "IntFunc";
            } else if (Type::is_same(node.type(), CharType::get())) {
                log_type = "CharFunc";
            }

            if (!_current_table->addFunc(func)) {
                _logger->logError(std::make_shared<ErrorLog>(node.line(), "func def duplicate", buaa::ERROR_IDENT_REDEF));
            } else {
                _logger->log(std::make_shared<SyntaxLog>(node.line(), _current_table->blockn(), func->ident(), log_type));
            }
        }
        virtual void visit(FuncFParamsNode& node) override {
            for (auto& [type, ident] : node.params()) {
                std::shared_ptr<Var> var;
                std::string log_type = "";
                if (auto ptr_t = dynamic_cast<PtrType*>(type); ptr_t) {
                    auto base_t = ptr_t->next();
                    if (Type::is_same(base_t, IntType::get())) {
                        var = Var::getIntPtr(ident, false);
                        log_type = "IntArray";
                    } else if (Type::is_same(base_t, CharType::get())) {
                        var = Var::getCharPtr(ident, false);
                        log_type = "CharArray";
                    }
                } else {
                    if (Type::is_same(type, IntType::get())) {
                        var = Var::getInt(ident, false);
                        log_type = "Int";
                    } else if (Type::is_same(type, CharType::get())) {
                        var = Var::getChar(ident, false);
                        log_type = "Char";
                    }
                }
                if (!_func_block->addVar(var)) {
                    _logger->logError(std::make_shared<ErrorLog>(node.line(), "var def duplicate", buaa::ERROR_IDENT_REDEF));
                } else {
                    _logger->log(std::make_shared<SyntaxLog>(node.line(), _func_block->blockn(), var->ident(), log_type));
                }
            }
        }
    public:
        DefChecker(std::shared_ptr<Logger> logger) :
            Checker(logger) {}
        void check(Type* type, DefNode& node, std::shared_ptr<SymbolTable> table) {
            _current_table = table;
            _type = type;
            node.accept(*this);
        }
        std::shared_ptr<BlockSymbolTable> check(FuncDefNode& node, std::shared_ptr<SymbolTable> table) {
            _current_table = table;
            node.accept(*this);
            return _func_block;
        }
    };
    // c
    class IdentChecker : public Checker {
    private:
        virtual void visit(LValNode& node) override {
            if (!_current_table->getVar(node.ident())) {
                _logger->logError(std::make_shared<ErrorLog>(node.line(), "lval ident not defined", buaa::ERROR_IDENT_UNDEF));
            }
        }
        virtual void visit(UnaryExpNode& node) override {
            if (!node.ident().empty()) {
                if (!_current_table->getFunc(node.ident())) {
                    _logger->logError(std::make_shared<ErrorLog>(node.line(), "unary ident not defined", buaa::ERROR_IDENT_UNDEF));
                }
            }
        }
    public:
        IdentChecker(std::shared_ptr<Logger> logger) :
            Checker(logger) {}
        void check(LValNode& node, std::shared_ptr<SymbolTable> table) {
            _current_table = table;
            node.accept(*this);
        }
        void check(UnaryExpNode& node, std::shared_ptr<SymbolTable> table) {
            _current_table = table;
            node.accept(*this);
        }
    };
    // d, e
    class ParamChecker : public Checker {
    private:
        class ValueAssertChecker : public Checker {
        private:
            virtual void visit(LValNode& node) override {
                auto var = _current_table->getVar(node.ident());
                if (!var) {
                    return ;
                }
                if (auto array_t = dynamic_cast<ArrayType*>(var->type()); array_t) {
                    if (!node.exp()) {
                        throw std::runtime_error("array ident without exp");
                    }
                }
            }
            virtual void visit(PrimaryExpNode& node) override {
                if (node.lval()) {
                    node.lval()->accept(*this);
                } else if (node.exp()) {
                    node.exp()->accept(*this);
                } else if (node.value()) {
                    ;   // value type, pass
                }
            }
            virtual void visit(UnaryExpNode& node) override {
                if (node.unary_exp()) {
                    node.unary_exp()->accept(*this);
                } else if (!node.ident().empty()) {
                    ;   // function call will only return value, pass
                } else if (node.primary_exp()) {
                    node.primary_exp()->accept(*this);
                }
            }
            virtual void visit(BinaryExpNode& node) override {
                node.left()->accept(*this);
                node.right()->accept(*this);
            }
        public:
            ValueAssertChecker() : Checker(nullptr) {}
            bool check(std::shared_ptr<ExpNode> node, std::shared_ptr<SymbolTable> table) {
                _current_table = table;
                try {
                    node->accept(*this);
                } catch (std::runtime_error& err) {
                    return false;
                }
                return true;
            }
        };
        class PtrAssertChecker : public Checker {
        private:
            PtrType* _ptr_t;
            virtual void visit(LValNode& node) override {
                auto var = _current_table->getVar(node.ident());
                if (!var) {
                    return ;
                }
                if (auto array_t = dynamic_cast<ArrayType*>(var->type()); array_t) {
                    if (node.exp()) {
                        throw std::runtime_error("array ident with exp");
                    }
                    if (!Type::is_same(array_t->type(), _ptr_t->next())) {
                        throw std::runtime_error("type not match");
                    }
                } else {
                    throw std::runtime_error("var ident");
                }
            }
            virtual void visit(PrimaryExpNode& node) override {
                if (node.lval()) {
                    node.lval()->accept(*this);
                } else if (node.exp()) {
                    node.exp()->accept(*this);
                } else if (node.value()) {
                    throw std::runtime_error("value type will not be ptr");
                }
            }
            virtual void visit(UnaryExpNode& node) override {
                if (node.unary_exp()) {
                    node.unary_exp()->accept(*this);
                } else if (!node.ident().empty()) {
                    throw std::runtime_error("function call only return value");
                } else if (node.primary_exp()) {
                    node.primary_exp()->accept(*this);
                }
            }
            virtual void visit(BinaryExpNode& node) override {
                node.left()->accept(*this);
                node.right()->accept(*this);
            }
        public:
            PtrAssertChecker() : Checker(nullptr) {}
            bool check(std::shared_ptr<ExpNode> node, PtrType* ptr_t, std::shared_ptr<SymbolTable> table) {
                _current_table = table;
                _ptr_t = ptr_t;
                try {
                    node->accept(*this);
                } catch (std::runtime_error& err) {
                    return false;
                }
                return true;
            }
        };
        std::shared_ptr<Func> _func;

        virtual void visit(UnaryExpNode& node) override {
            _func = _current_table->getFunc(node.ident());
            if (!_func) {
                return ;
            }
            if (node.func_rparams()) {
                node.func_rparams()->accept(*this);
            }
        }
        virtual void visit(FuncRParamsNode& node) override {
            auto func_params = _func->params();
            if (func_params.size() != node.nodes().size()) {
                _logger->logError(std::make_shared<ErrorLog>(node.line(), "func param count no match", buaa::ERROR_FUNC_PARAM_COUNT_NOT_MATCH));
                return ;
            }
            for (size_t i = 0; i < node.nodes().size(); i++) {
                auto [type, ident] = func_params[i];
                auto exp = node.nodes()[i];
                if (auto ptr_t = dynamic_cast<PtrType*>(type); ptr_t) {
                    auto checker = PtrAssertChecker();
                    if (!checker.check(exp, ptr_t, _current_table)) {
                        _logger->logError(std::make_shared<ErrorLog>(node.line(), "func param type no match", buaa::ERROR_FUNC_PARAM_TYPE_NOT_MATCH));
                    }
                } else {
                    auto checker = ValueAssertChecker();
                    if (!checker.check(exp, _current_table)) {
                        _logger->logError(std::make_shared<ErrorLog>(node.line(), "func param type no match", buaa::ERROR_FUNC_PARAM_TYPE_NOT_MATCH));
                    }
                }
            }
        }
    public:
        ParamChecker(std::shared_ptr<Logger> logger) :
            Checker(logger) {}
        void check(UnaryExpNode& node, std::shared_ptr<SymbolTable> table) {
            _current_table = table;
            if (node.ident().empty()) {
                return ;
            }
            node.accept(*this);
        }
    };
    // f, g
    class ReturnChecker : public Checker {
    private:
        virtual void visit(ForStmtNode& node) override {
            node.stmt()->accept(*this);
        }
        virtual void visit(IfStmtNode& node) override {
            if (node.if_stmt()) {
                node.if_stmt()->accept(*this);
            }
            if (node.else_stmt()) {
                node.else_stmt()->accept(*this);
            }
        }
        virtual void visit(ReturnStmtNode& node) override {
            if (node.exp()) {
                _logger->logError(std::make_shared<ErrorLog>(node.line(), "void return", buaa::ERROR_VOID_FUNC_RETURN));
            }
        }
        virtual void visit(BlockStmtNode& node) override {
            node.block()->accept(*this);
        }
        virtual void visit(BlockItemNode& node) override {
            if (node.stmt()) {
                node.stmt()->accept(*this);
            }
        }
        virtual void visit(BlockNode& node) override {
            for (auto& item : node.items()) {
                item->accept(*this);
            }
        }
        // block will only be visited if check for void return types
        virtual void visit(FuncDefNode& node) override {
            if (Type::is_same(node.type(), VoidType::get())) {
                node.block()->accept(*this);
            } else {
                auto block = node.block();
                auto items = block->items();
                if (items.empty()) {
                    _logger->logError(std::make_shared<ErrorLog>(block->line(), "no return", buaa::ERROR_FUNC_NO_RETURN));
                    return ;
                }
                auto last_item = items[items.size() - 1];
                if (auto ret_stmt = std::dynamic_pointer_cast<ReturnStmtNode>(last_item->stmt()); ret_stmt) {
                    if (!ret_stmt->exp()) {
                        _logger->logError(std::make_shared<ErrorLog>(block->line(), "no return", buaa::ERROR_FUNC_NO_RETURN));
                    }
                } else {
                    _logger->logError(std::make_shared<ErrorLog>(block->line(), "no return", buaa::ERROR_FUNC_NO_RETURN));
                }
            }
        }
        virtual void visit(MainNode& node) override {
            auto block = node.block();
            auto items = block->items();
            if (items.empty()) {
                _logger->logError(std::make_shared<ErrorLog>(block->line(), "no return", buaa::ERROR_FUNC_NO_RETURN));
                return ;
            }
            auto last_item = items[items.size() - 1];
            if (auto ret_stmt = std::dynamic_pointer_cast<ReturnStmtNode>(last_item->stmt()); ret_stmt) {
                if (!ret_stmt->exp()) {
                    _logger->logError(std::make_shared<ErrorLog>(block->line(), "no return", buaa::ERROR_FUNC_NO_RETURN));
                }
            } else {
                _logger->logError(std::make_shared<ErrorLog>(block->line(), "no return", buaa::ERROR_FUNC_NO_RETURN));
            }
        }
    public:
        ReturnChecker(std::shared_ptr<Logger> logger) :
            Checker(logger) {}
        void check(FuncDefNode& node, std::shared_ptr<SymbolTable> table) {
            _current_table = table;
            node.accept(*this);
        }
        void check(MainNode& node, std::shared_ptr<SymbolTable> table) {
            _current_table = table;
            node.accept(*this);
        }
    };
    // h
    class AssignChecker : public Checker {
    private:
        virtual void visit(LValNode& node) override {
            auto var = _current_table->getVar(node.ident());
            if (!var) {
                return ;
            }
            if (var->is_const()) {
                _logger->logError(std::make_shared<ErrorLog>(node.line(), "const modify", buaa::ERROR_CONST_MODIFY));
            }
        }
        virtual void visit(AssignStmtNode& node) override {
            node.lval()->accept(*this);
        }
    public:
        AssignChecker(std::shared_ptr<Logger> logger) :
            Checker(logger) {}
        void check(AssignStmtNode& node, std::shared_ptr<SymbolTable> table) {
            _current_table = table;
            node.accept(*this);
        }
    };
    // l
    class PrintfChecker : public Checker {
    private:
        virtual void visit(PrintfStmtNode& node) override {
            auto count = 0;
            for (auto iter = node.fmt().find("%d"); iter != -1; iter = node.fmt().find("%d", iter + 1)) {
                count++;
            }
            for (auto iter = node.fmt().find("%c"); iter != -1; iter = node.fmt().find("%c", iter + 1)) {
                count++;
            }
            if (count != node.exps().size()) {
                _logger->logError(std::make_shared<ErrorLog>(node.line(), "printf", buaa::ERROR_PRINTF_PARAM_COUNT_NOT_MATCH));
            }
        }
    public:
        PrintfChecker(std::shared_ptr<Logger> logger) :
            Checker(logger) {}
        void check(PrintfStmtNode& node, std::shared_ptr<SymbolTable> table) {
            _current_table = table;
            node.accept(*this);
        }
    };
    // m
    class BlockChecker : public Checker {
    private:
        virtual void visit(ContinueStmtNode& node) override {
            _logger->logError(std::make_shared<ErrorLog>(node.line(), "continue", buaa::ERROR_ITER_IDENT_MISUSE));
        }
        virtual void visit(BreakStmtNode& node) override {
            _logger->logError(std::make_shared<ErrorLog>(node.line(), "break", buaa::ERROR_ITER_IDENT_MISUSE));
        }
        virtual void visit(IfStmtNode& node) override {
            node.if_stmt()->accept(*this);
            if (node.else_stmt()) {
                node.else_stmt()->accept(*this);
            }
        }
        virtual void visit(BlockStmtNode& node) override {
            node.block()->accept(*this);
        }
        virtual void visit(BlockItemNode& node) override {
            if (node.stmt()) {
                node.stmt()->accept(*this);
            }
        }
        virtual void visit(BlockNode& node) override {
            for (auto& item : node.items()) {
                item->accept(*this);
            }
        }
    public:
        BlockChecker(std::shared_ptr<Logger> logger) :
            Checker(logger) {}
        void check(BlockNode& node, std::shared_ptr<SymbolTable> table) {
            _current_table = table;
            node.accept(*this);
        }
    };
    virtual void visit(CompNode& node) override;
    virtual void visit(DeclNode& node) override;
    virtual void visit(DefNode& node) override;
    virtual void visit(InitValNode& node) override;
    virtual void visit(UnaryExpNode& node) override;
    virtual void visit(BinaryExpNode& node) override;
    virtual void visit(PrimaryExpNode& node) override;
    virtual void visit(LValNode& node) override;
    virtual void visit(ValueNode& node) override;
    virtual void visit(FuncDefNode& node) override;
    virtual void visit(MainNode& node) override;
    virtual void visit(BlockNode& node) override;
    virtual void visit(BlockItemNode& node) override;
    virtual void visit(AssignStmtNode& node) override;
    virtual void visit(ReturnStmtNode& node) override;
    virtual void visit(PrintfStmtNode& node) override;
    virtual void visit(BreakStmtNode& node) override;
    virtual void visit(ContinueStmtNode& node) override;
    virtual void visit(ExpStmtNode& node) override;
    virtual void visit(ForStmtNode& node) override;
    virtual void visit(IfStmtNode& node) override;
    virtual void visit(BlockStmtNode& node) override;
    virtual void visit(RValNode& node) override;
    virtual void visit(FuncRParamsNode& node) override;
    virtual void visit(FuncFParamsNode& node) override;
public:
    SyntaxChecker(std::shared_ptr<Logger> logger);
    std::shared_ptr<GlobalSymbolTable> check(std::shared_ptr<CompNode> comp_unit);
};

}

}

#endif