#include "syntax_checker.hpp"
#include "ast.hpp"
#include "symbol_table.hpp"
#include <memory>

namespace blang {

namespace frontend {

SyntaxChecker::SyntaxChecker(std::shared_ptr<Logger> logger) :
    _logger(logger) {
    
}

std::shared_ptr<GlobalSymbolTable> SyntaxChecker::check(std::shared_ptr<CompNode> comp_unit) {
    _global_table = std::make_shared<GlobalSymbolTable>();
    _current_table = _global_table;

    comp_unit->accept(*this);

    return _global_table;
}

void SyntaxChecker::visit(CompNode& node) {
    if (node.comp()) {
        node.comp()->accept(*this);
    }

    if (node.decl()) {
        node.decl()->accept(*this);
    } else if (node.func_def()) {
        node.func_def()->accept(*this);
    } else if (node.main_func_def()) {
        _global_table->main_node() = node.main_func_def();
        node.main_func_def()->accept(*this);
    }
}

void SyntaxChecker::visit(DeclNode& node) {
    auto checker = DefChecker(_logger);
    for (auto& def : node.defs()) {
        checker.check(node.type(), *def, _current_table);
        def->accept(*this);
    }
}

void SyntaxChecker::visit(DefNode& node) {
    if (node.array_exp()) {
        node.array_exp()->accept(*this);
    }
    if (node.init_val()) {
        node.init_val()->accept(*this);
    }
}

void SyntaxChecker::visit(InitValNode& node) {
    if (node.exp()) {
        node.exp()->accept(*this);
    } else if (!node.exps().empty()) {
        for (auto& exp : node.exps()) {
            exp->accept(*this);
        }
    }
}

void SyntaxChecker::visit(UnaryExpNode& node) {
    if (node.unary_exp()) {
        node.unary_exp()->accept(*this);
    } else if (node.primary_exp()) {
        node.primary_exp()->accept(*this);
    } else if (!node.ident().empty()) {
        auto ident_checker = IdentChecker(_logger);
        ident_checker.check(node, _current_table);
        auto param_checker = ParamChecker(_logger);
        param_checker.check(node, _current_table);
        if (node.func_rparams()) {
            node.func_rparams()->accept(*this);
        }
    }
}

void SyntaxChecker::visit(FuncRParamsNode& node) {
    for (auto& exp : node.nodes()) {
        exp->accept(*this);
    }
}

void SyntaxChecker::visit(PrimaryExpNode& node) {
    if (node.exp()) {
        node.exp()->accept(*this);
    } else if (node.lval()) {
        node.lval()->accept(*this);
    } else if (node.value()) {
        node.value()->accept(*this);
    }
}

void SyntaxChecker::visit(LValNode& node) {
    auto checker = IdentChecker(_logger);
    checker.check(node, _current_table);
    if (node.exp()) {
        node.exp()->accept(*this);
    }
}

void SyntaxChecker::visit(ValueNode& node) {}

void SyntaxChecker::visit(BinaryExpNode& node) {
    node.left()->accept(*this);
    node.right()->accept(*this);
}

void SyntaxChecker::visit(FuncDefNode& node) {
    auto def_checker = DefChecker(_logger);
    auto block_table = def_checker.check(node, _current_table);

    if (node.params()) {
        node.params()->accept(*this);
    }

    auto tmp = _current_table;
    _current_table = block_table;
    node.block()->accept(*this);

    auto block_checker = BlockChecker(_logger);
    block_checker.check(*node.block(), _current_table);

    auto return_checker = ReturnChecker(_logger);
    return_checker.check(node, _current_table);

    _current_table = tmp;
}

void SyntaxChecker::visit(FuncFParamsNode& node) {}

void SyntaxChecker::visit(MainNode& node) {
    auto tmp = _current_table;
    _current_table = std::make_shared<BlockSymbolTable>(_current_table);
    node.block()->accept(*this);

    auto block_checker = BlockChecker(_logger);
    block_checker.check(*node.block(), _current_table);

    auto return_checker = ReturnChecker(_logger);
    return_checker.check(node, _current_table);

    _current_table = tmp;
}

void SyntaxChecker::visit(BlockNode& node) {
    for (auto& item : node.items()) {
        item->accept(*this);
    }
}

void SyntaxChecker::visit(BlockItemNode& node) {
    if (node.decl()) {
        node.decl()->accept(*this);
    } else if (node.stmt()) {
        node.stmt()->accept(*this);
    }
}

void SyntaxChecker::visit(AssignStmtNode& node) {
    auto checker = AssignChecker(_logger);
    checker.check(node, _current_table);
    node.lval()->accept(*this);
    node.rval()->accept(*this);
}

void SyntaxChecker::visit(RValNode& node) {
    if (node.exp()) {
        node.exp()->accept(*this);
    }
}

void SyntaxChecker::visit(ReturnStmtNode& node) {
    if (node.exp()) {
        node.exp()->accept(*this);
    }
}

void SyntaxChecker::visit(PrintfStmtNode& node) {
    auto checker = PrintfChecker(_logger);
    checker.check(node, _current_table);

    for (auto& exp : node.exps()) {
        exp->accept(*this);
    }
}

void SyntaxChecker::visit(BreakStmtNode& node) {}

void SyntaxChecker::visit(ContinueStmtNode& node) {}

void SyntaxChecker::visit(ExpStmtNode& node) {
    if (node.exp()) {
        node.exp()->accept(*this);
    }
}

void SyntaxChecker::visit(ForStmtNode& node) {
    if (node.for_in()) {
        node.for_in()->accept(*this);
    }
    if (node.cond()) {
        node.cond()->accept(*this);
    }
    if (node.for_out()) {
        node.for_out()->accept(*this);
    }
    node.stmt()->accept(*this);
}

void SyntaxChecker::visit(IfStmtNode& node) {
    node.cond()->accept(*this);
    node.if_stmt()->accept(*this);
    if (node.else_stmt()) {
        node.else_stmt()->accept(*this);
    }
}

void SyntaxChecker::visit(BlockStmtNode& node) {
    auto tmp = _current_table;
    _current_table = std::make_shared<BlockSymbolTable>(_current_table);
    node.block()->accept(*this);
    _current_table = tmp;
}

}

}