#include "ast.hpp"
#include "visitor.hpp"

namespace blang {

namespace entities {

// BinaryExpNode
void BinaryExpNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// UnaryExpNode
void UnaryExpNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// InitValNode
void InitValNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// DefNode
void DefNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// DeclNode
void DeclNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// FuncRParamsNode
void FuncRParamsNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

void ValueNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// RValNode
void RValNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// PrimaryExpNode
void PrimaryExpNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// LValNode
void LValNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// AssignStmtNode
void AssignStmtNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// PrintfStmtNode
void PrintfStmtNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// ExpStmtNode
void ExpStmtNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// BlockStmtNode
void BlockStmtNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// IfStmtNode
void IfStmtNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// ForStmtNode
void ForStmtNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// BreakStmtNode
void BreakStmtNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// ContinueStmtNode
void ContinueStmtNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// ReturnStmtNode
void ReturnStmtNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// BlockItemNode
void BlockItemNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// BlockNode
void BlockNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// FuncFParamsNode
void FuncFParamsNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// FuncDefNode
void FuncDefNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// MainNode
void MainNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

// CompNode
void CompNode::accept(Visitor& visitor) {
    visitor.visit(*this);
}

}

}