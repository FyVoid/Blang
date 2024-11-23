#ifndef BLANG_IR_GENERATOR_H
#define BLANG_IR_GENERATOR_H

#include "ast.hpp"
#include "evaluator.hpp"
#include "ir.hpp"
#include "logger.hpp"
#include "symbol_table.hpp"
#include "visitor.hpp"
#include <memory>
#include <string>

namespace blang {
namespace backend {

using namespace entities;
using namespace tools;

class IrGenerator : public Visitor {
private:
    std::shared_ptr<Logger> _logger;
    std::shared_ptr<GlobalSymbolTable> _global_table;
    std::shared_ptr<SymbolTable> _current_table;
    std::shared_ptr<IrModule> _module;
    std::shared_ptr<IrFactory> _factory;
    std::vector<std::string> _for_end_labels = {};
    std::vector<std::string> _for_out_labels = {};
    static int32_t evaluate(ExpNode& node, std::shared_ptr<SymbolTable> current_table) {
        static Evaluator evaluator{};
        try {
            return evaluator.evaluate(node, current_table);
        } catch (std::runtime_error err) {
            return -1;
        }
    }
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
    void setGlobalVar();
    void setFunction();
    void setMain();
public:
    IrGenerator(std::shared_ptr<Logger> logger);
    std::shared_ptr<IrModule> gen(std::shared_ptr<GlobalSymbolTable> checked_table);
};

}
}

#endif