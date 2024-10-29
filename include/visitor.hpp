#ifndef BLANG_VISITOR_H
#define BLANG_VISITOR_H

#include "ast.hpp"
namespace blang {

namespace entities {

class Visitor {
protected:
    Visitor() {}
public:
    virtual void visit(BinaryExpNode& node) {}
    virtual void visit(UnaryExpNode& node) {}
    virtual void visit(InitValNode& node) {}
    virtual void visit(DefNode& node) {}
    virtual void visit(DeclNode& node) {}
    virtual void visit(FuncRParamsNode& node) {}
    virtual void visit(ValueNode& node) {}
    virtual void visit(RValNode& node) {}
    virtual void visit(PrimaryExpNode& node) {}
    virtual void visit(LValNode& node) {}
    virtual void visit(AssignStmtNode& node) {}
    virtual void visit(PrintfStmtNode& node) {}
    virtual void visit(ExpStmtNode& node) {}
    virtual void visit(BlockStmtNode& node) {}
    virtual void visit(IfStmtNode& node) {}
    virtual void visit(ForStmtNode& node) {}
    virtual void visit(BreakStmtNode& node) {}
    virtual void visit(ContinueStmtNode& node) {}
    virtual void visit(ReturnStmtNode& node) {}
    virtual void visit(BlockItemNode& node) {}
    virtual void visit(BlockNode& node) {}
    virtual void visit(FuncFParamsNode& node) {}
    virtual void visit(FuncDefNode& node) {}
    virtual void visit(MainNode& node) {}
    virtual void visit(CompNode& node) {}
};

}

}

#endif