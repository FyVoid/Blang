#include "ir_generator.hpp"
#include "ast.hpp"
#include "ir.hpp"
#include "symbol_table.hpp"
#include "type.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace blang {
namespace backend {

IrGenerator::IrGenerator(std::shared_ptr<Logger> logger) {
    _logger = logger;
}

std::shared_ptr<IrModule> IrGenerator::gen(std::shared_ptr<GlobalSymbolTable> checked_table) {
    _global_table = checked_table;
    _current_table = checked_table;

    _module = std::make_shared<IrModule>();
    _factory = std::make_shared<IrFactory>(_module);

    setGlobalVar();
    setFunction();
    setMain();

    return _module;
}

void IrGenerator::setGlobalVar() {
    for (auto& var : _global_table->getVars()) {
        auto type = var->type();
        std::shared_ptr<Value> value;
        std::shared_ptr<PtrValue> ptr;
        auto ident = var->ident() + "_" + _factory->next_reg();
        if (Type::is_same(type, IntType::get())) {
            value = std::make_shared<IntConstValue>(*(var->get<int32_t>()));
            ptr = std::make_shared<PtrValue>(var->type(), ident);
        } else if (Type::is_same(type, CharType::get())) {
            value = std::make_shared<CharConstValue>(*(var->get<char>()));
            ptr = std::make_shared<PtrValue>(var->type(), ident);
        } else if (auto array_t = dynamic_cast<ArrayType*>(type); array_t) {
            if (Type::is_same(array_t->type(), IntType::get())) {
                std::vector<std::shared_ptr<Value>> vec{};
                auto array = var->get<int32_t>();
                for (size_t i = 0; i < array_t->length(); i++) {
                    vec.push_back(std::make_shared<IntConstValue>(array[i]));
                }
                value = std::make_shared<ArrayValue>(array_t, vec);
                ptr = std::make_shared<PtrValue>(var->type(), ident);
            } else if (Type::is_same(array_t->type(), CharType::get())) {
                std::vector<std::shared_ptr<Value>> vec{};
                auto array = var->get<char>();
                for (size_t i = 0; i < array_t->length(); i++) {
                    vec.push_back(std::make_shared<CharConstValue>(array[i]));
                }
                value = std::make_shared<ArrayValue>(array_t, vec);
                ptr = std::make_shared<PtrValue>(var->type(), ident);
            }
        }

        var->value() = ptr;
        _factory->addDefInstruct(var->is_const(), ptr, value);
    }
}

void IrGenerator::setFunction() {
    for (auto& func : _global_table->getFuncs()) {
        _factory->addFunction(func->return_type(), func->ident(), func->params());
        func->function() = _module->current_function();
        func->node()->accept(*this);
    }
}

void IrGenerator::setMain() {
    auto main = _global_table->main_node();
    _factory->addFunction(IntType::get(), "main", {});
    main->accept(*this);
}

void IrGenerator::visit(CompNode& node) {}

void IrGenerator::visit(DeclNode& node) {
    auto decl_type = node.type();
    auto is_const = node.is_const();
    for (auto& def : node.defs()) {
        // add var
        std::shared_ptr<Var> var;
        std::shared_ptr<Value> init;
        if (def->array_exp()) {
            auto length = evaluate(*def->array_exp(), _current_table);
            if (Type::is_same(decl_type, IntType::get())) {
                var = Var::getIntArray(def->ident(), def->is_const(), length);
                if (def->is_const()) {
                    auto init_val = getInitVal<int32_t>(*def->init_val());
                    memcpy(var->get<int32_t>(), init_val.data(), sizeof(int32_t) * std::min(length, static_cast<int32_t>(init_val.size())));
                }
            } else if (Type::is_same(decl_type, CharType::get())) {
                var = Var::getCharArray(def->ident(), def->is_const(), length);
                if (def->is_const()) {
                    auto init_val = getInitVal<char>(*def->init_val());
                    memcpy(var->get<char>(), init_val.data(), sizeof(char) * std::min(length, static_cast<int32_t>(init_val.size())));
                }
            }
        } else {
            if (Type::is_same(decl_type, IntType::get())) {
                var = Var::getInt(def->ident(), def->is_const());
                if (def->is_const()) {
                    auto init_val = getInitVal<int32_t>(*def->init_val());
                    memcpy(var->get<int32_t>(), init_val.data(), sizeof(int32_t));
                }
            } else if (Type::is_same(decl_type, CharType::get())) {
                var = Var::getChar(def->ident(), def->is_const());
                if (def->is_const()) {
                    auto init_val = getInitVal<char>(*def->init_val());
                    memcpy(var->get<char>(), init_val.data(), sizeof(char));
                }
            }
        }
        _current_table->addVar(var);

        // add instruction
        std::shared_ptr<PtrValue> ptr;
        auto type = var->type();
        auto ident = var->ident() + "_" + _factory->next_reg();
        if (Type::is_same(type, IntType::get())) {
            ptr = std::make_shared<PtrValue>(var->type(), ident);
        } else if (Type::is_same(type, CharType::get())) {
            ptr = std::make_shared<PtrValue>(var->type(), ident);
        } else if (auto array_t = dynamic_cast<ArrayType*>(type); array_t) {
            if (Type::is_same(array_t->type(), IntType::get())) {
                std::vector<std::shared_ptr<Value>> vec{};
                auto array = var->get<int32_t>();
                for (size_t i = 0; i < array_t->length(); i++) {
                    vec.push_back(std::make_shared<IntConstValue>(array[i]));
                }
                ptr = std::make_shared<PtrValue>(var->type(), ident);
            } else if (Type::is_same(array_t->type(), CharType::get())) {
                std::vector<std::shared_ptr<Value>> vec{};
                auto array = var->get<char>();
                for (size_t i = 0; i < array_t->length(); i++) {
                    vec.push_back(std::make_shared<CharConstValue>(array[i]));
                }
                ptr = std::make_shared<PtrValue>(var->type(), ident);
            }
        }

        var->value() = ptr;
        _factory->addAllocaInstruct(ptr);

        if (def->init_val()) {
            if (Type::is_same(type, IntType::get())) {
                def->init_val()->exp()->accept(*this);
                auto result = _module->current_block()->last()->reg();
                _factory->addStoreInstruct(result, ptr);
            } else if (Type::is_same(type, CharType::get())) {
                def->init_val()->exp()->accept(*this);
                auto result = _module->current_block()->last()->reg();
                auto reg = std::make_shared<CharValue>(_factory->next_reg());
                _factory->addTruncInstruct(reg, result);
                _factory->addStoreInstruct(reg, ptr);
            } else if (auto array_t = dynamic_cast<ArrayType*>(type); array_t) {
                if (Type::is_same(array_t->type(), IntType::get())) {
                    int iter = 0;
                    for (auto& exp : def->init_val()->exps()) {
                        exp->accept(*this);
                        auto result = _module->current_block()->last()->reg();
                        auto gep_ptr = std::make_shared<PtrValue>(array_t, _factory->next_reg());
                        auto elem = std::make_shared<IntConstValue>(0);
                        auto offset = std::make_shared<IntConstValue>(iter);
                        iter++;
                        _factory->addGepInstruct(gep_ptr, ptr, elem, offset);
                        _factory->addStoreInstruct(result, gep_ptr);
                    }
                } else if (Type::is_same(array_t->type(), CharType::get())) {
                    int iter = 0;
                    for (auto& exp : def->init_val()->exps()) {
                        exp->accept(*this);
                        auto result = _module->current_block()->last()->reg();
                        auto gep_ptr = std::make_shared<PtrValue>(array_t, _factory->next_reg());
                        auto elem = std::make_shared<IntConstValue>(0);
                        auto offset = std::make_shared<IntConstValue>(iter);
                        iter++;
                        _factory->addGepInstruct(gep_ptr, ptr, elem, offset);
                        auto reg = std::make_shared<CharValue>(_factory->next_reg());
                        _factory->addSextInstruct(reg, result);
                        result = _module->current_block()->last()->reg();
                        _factory->addStoreInstruct(result, gep_ptr);
                    }
                }
            }
        }
    }
}

void IrGenerator::visit(DefNode& node) {}

void IrGenerator::visit(InitValNode& node) {}

void IrGenerator::visit(UnaryExpNode& node) {
    if (node.unary_exp()) {
        node.unary_exp()->accept(*this);
    } else if (node.primary_exp()) {
        node.primary_exp()->accept(*this);
    } else if (!node.ident().empty()) {
        auto func = _current_table->getFunc(node.ident());
        std::shared_ptr<PtrValue> ptr;
        if (Type::is_same(func->return_type(), VoidType::get())) {
            ptr = nullptr;
        } else {
            ptr = std::make_shared<PtrValue>(func->return_type(), _factory->next_reg());
        }

        std::vector<std::shared_ptr<Value>> params{};
        if (node.func_rparams()) {
            for (auto& param : node.func_rparams()->nodes()) {
                param->accept(*this);
                params.push_back(_module->current_block()->last()->reg());
            }
        }

        _factory->addCallInstruct(ptr, func->function(), params);
    }
}

void IrGenerator::visit(BinaryExpNode& node) {
    node.left()->accept(*this);
    auto left = _module->current_block()->last()->reg();
    if (Type::is_same(left->getType(), CharType::get())) {
        auto reg = std::make_shared<IntValue>(_factory->next_reg());
        _factory->addSextInstruct(reg, left);
        left = _module->current_block()->last()->reg();
    }

    node.right()->accept(*this);
    auto right = _module->current_block()->last()->reg();
    if (Type::is_same(right->getType(), CharType::get())) {
        auto reg = std::make_shared<IntValue>(_factory->next_reg());
        _factory->addSextInstruct(reg, right);
        right = _module->current_block()->last()->reg();
    }
    
    auto regn = _factory->next_reg();
    auto int_reg = std::make_shared<IntValue>(regn);
    auto bool_reg = std::make_shared<BoolValue>(regn);
    switch (node.op()) {
        case OP_ADD: {
            _factory->addAddInstruct(int_reg, left, right);
        }
        break;
        case OP_MINUS: {
            _factory->addSubInstruct(int_reg, left, right);
        }
        break;
        case OP_MUL: {
            _factory->addMulInstruct(int_reg, left, right);
        }
        break;
        case OP_DIV: {
            _factory->addDivInstruct(int_reg, left, right);
        }
        break;
        case OP_MOD: {
            _factory->addModInstruct(int_reg, left, right);
        }
        break;
        case OP_AND: {
            _factory->addAndInstruct(bool_reg, left, right);
        }
        break;
        case OP_OR: {
            _factory->addOrInstruct(bool_reg, left, right);
        }
        break;
        case OP_EQ: {
            _factory->addEqInstruct(bool_reg, left, right);
        }
        break;
        case OP_NEQ: {
            _factory->addNeqInstruct(bool_reg, left, right);
        }
        break;
        case OP_GE: {
            _factory->addGeInstruct(bool_reg, left, right);
        }
        break;
        case OP_GT: {
            _factory->addGtInstruct(bool_reg, left, right);
        }
        break;
        case OP_LE: {
            _factory->addLeInstruct(bool_reg, left, right);
        }
        break;
        case OP_LT: {
            _factory->addLtInstruct(bool_reg, left, right);
        }
        break;
        default:
        break;
    }
}

void IrGenerator::visit(PrimaryExpNode& node) {
    if (node.exp()) {
        node.exp()->accept(*this);
    } else if (node.lval()) {
        node.lval()->accept(*this);
    } else if (node.value()) {
        node.value()->accept(*this);
    }
}

void IrGenerator::visit(LValNode& node) {
    auto var = _current_table->getVar(node.ident());
    if (auto array_t = dynamic_cast<ArrayType*>(var->type()); array_t) {
        node.exp()->accept(*this);
        auto offset = _module->current_block()->last()->reg();
        auto result = std::make_shared<PtrValue>(array_t->type(), _factory->next_reg());
        auto elem = std::make_shared<IntConstValue>(0);
        _factory->addGepInstruct(result, var->value(), elem, std::static_pointer_cast<IntConstValue>(offset));
    } else {
        if (Type::is_same(var->type(), IntType::get())) {
            _factory->addLoadInstruct(var->value(), std::make_shared<IntValue>(_factory->next_reg()));
        } else if (Type::is_same(var->type(), CharType::get())) {
            _factory->addLoadInstruct(var->value(), std::make_shared<CharValue>(_factory->next_reg()));
        }
    }
}

void IrGenerator::visit(ValueNode& node) {
    if (Type::is_same(node.type(), IntType::get())) {
        auto value = std::make_shared<IntConstValue>(node.get<int32_t>());
        auto result = std::make_shared<IntValue>(_factory->next_reg());
        _factory->addAddInstruct(result, value, std::make_shared<IntConstValue>(0));
    } else if (Type::is_same(node.type(), CharType::get())) {
        auto value = std::make_shared<CharConstValue>(node.get<char>());
        auto result = std::make_shared<CharValue>(_factory->next_reg());
        _factory->addAddInstruct(result, value, std::make_shared<CharConstValue>(0));
    }
}

void IrGenerator::visit(FuncDefNode& node) {
    auto tmp = _current_table;
    _current_table = std::make_shared<BlockSymbolTable>(_current_table);
    node.block()->accept(*this);
    _current_table = tmp;
}

void IrGenerator::visit(MainNode& node) {
    auto tmp = _current_table;
    _current_table = std::make_shared<BlockSymbolTable>(_current_table);
    node.block()->accept(*this);
    _current_table = tmp;
}

void IrGenerator::visit(BlockNode& node) {
    for (auto& block : node.items()) {
        block->accept(*this);
    }
}

void IrGenerator::visit(BlockItemNode& node) {
    if (node.decl()) {
        node.decl()->accept(*this);
    } else if (node.stmt()) {
        node.stmt()->accept(*this);
    }
}

void IrGenerator::visit(AssignStmtNode& node) {
    auto var = _current_table->getVar(node.lval()->ident());
    auto reg = var->value();
    node.rval()->accept(*this);
    auto value = _module->current_block()->last()->reg();

    if (!Type::is_same(reg->getType(), value->getType())) {
        if (Type::is_same(reg->getType(), CharType::get())) {
            auto result = std::make_shared<CharValue>(_factory->next_reg());
            _factory->addTruncInstruct(result, value);
            value = result;
        } else if (Type::is_same(reg->getType(), IntType::get())) {
            auto result = std::make_shared<IntValue>(_factory->next_reg());
            _factory->addSextInstruct(result, value);
            value = result;
        }
    }

    _factory->addStoreInstruct(value, std::dynamic_pointer_cast<PtrValue>(reg));
}

void IrGenerator::visit(ReturnStmtNode& node) {
    if (node.exp()) {
        node.exp()->accept(*this);
        auto ret_value = _module->current_block()->last()->reg();
        if (Type::is_same(_module->current_function()->ret_type(), CharType::get())) {
            if (Type::is_same(ret_value->getType(), IntType::get())) {
                auto result = std::make_shared<CharValue>(_factory->next_reg());
                _factory->addTruncInstruct(result, ret_value);
                ret_value = result;
            }
        } else if (Type::is_same(_module->current_function()->ret_type(), IntType::get())) {
            if (Type::is_same(ret_value->getType(), CharType::get())) {
                auto result = std::make_shared<IntValue>(_factory->next_reg());
                _factory->addSextInstruct(result, ret_value);
                ret_value = result;
            }
        }
        _factory->addRetInstruct(ret_value);
    } else {
        _factory->addRetInstruct(nullptr);
    }
}

void IrGenerator::visit(PrintfStmtNode& node) {
    auto fmt = node.fmt();
    auto params = node.exps();
    auto param_count = 0;
    std::shared_ptr<ArrayValue> str_array = nullptr;
    std::vector<std::shared_ptr<Value>> str_vec{};
    size_t pos = 0;
    while (pos < fmt.size()) {
        auto d_next = fmt.find("%d", pos);
        auto c_next = fmt.find("%c", pos);
        auto next = pos;
        if (std::min(d_next, c_next) == std::string::npos) {
            next = fmt.size();
        }
        else if (d_next < c_next) {
            next = d_next;
        } else {
            next = c_next;
        }
        str_vec = {};
        auto type = ArrayType::get(CharType::get(), next - pos + 1);
        if (pos < next) {
            while (pos < next) {
                str_vec.push_back(std::make_shared<CharConstValue>(fmt[pos]));
                pos++;
            }
            str_vec.push_back(std::make_shared<CharConstValue>('\0'));
            str_array = std::make_shared<ArrayValue>(type, str_vec);
            auto alloca = std::make_shared<PtrValue>(type, _factory->next_reg());
            _factory->addAllocaInstruct(alloca);
            _factory->addStoreInstruct(str_array, alloca);
            _factory->addCallExternalInstruct(nullptr, "putstr", {alloca});
        }
        if (d_next < c_next && d_next != std::string::npos) {
            params[param_count++]->accept(*this);
            auto reg = _module->current_block()->last()->reg();
            _factory->addCallExternalInstruct(nullptr, "putint", {reg});
            pos = next + 2;
        } else if (c_next < d_next && c_next != std::string::npos) {
            params[param_count++]->accept(*this);
            auto reg = _module->current_block()->last()->reg();
            auto tmp = reg;
            reg = std::make_shared<CharValue>(_factory->next_reg());
            _factory->addTruncInstruct(reg, tmp);
            _factory->addCallExternalInstruct(nullptr, "putchar", {reg});
            pos = next + 2;
        }
    }
}

void IrGenerator::visit(BreakStmtNode& node) {
    _factory->addBrInstruct(_for_end_label);
}

void IrGenerator::visit(ContinueStmtNode& node) {
    _factory->addBrInstruct(_for_in_label);
}

void IrGenerator::visit(ExpStmtNode& node) {
    if (node.exp()) {
        node.exp()->accept(*this);
    }
}

void IrGenerator::visit(ForStmtNode& node) {
    auto forn = _factory->next_block();
    _factory->addBrInstruct("for_entry" + forn);
    _module->current_function()->addBlock("for_entry" + forn);
    auto for_entry = _module->current_block();
    if (node.for_in()) {
        node.for_in()->accept(*this);
    }
    _factory->addBrInstruct("for_in" + forn);
    _for_in_label = "for_in" + forn;
    _for_end_label = "for_end" + forn;

    _module->current_function()->addBlock("for_in" + forn);
    auto for_in = _module->current_function()->current_block();
    if (node.cond()) {
        node.cond()->accept(*this);
        auto result = _module->current_block()->last()->reg();
        if (!Type::is_same(result->getType(), BoolType::get())) {
            auto tmp = result;
            result = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addTruncInstruct(result, tmp);
        }
        _factory->addCondBrInstruct(result, "for_body" + forn, "for_end" + forn);
    } else {
        _factory->addBrInstruct("for_body" + forn);
    }

    _module->current_function()->addBlock("for_body" + forn);
    auto for_body = _module->current_function()->current_block();
    node.stmt()->accept(*this);
    if (node.for_out()) {
        node.for_out()->accept(*this);
    }
    _factory->addBrInstruct("for_in" + forn);

    _module->current_function()->addBlock("for_end" + forn);
}

void IrGenerator::visit(IfStmtNode& node) {
    auto ifn = _factory->next_block();
    _factory->addBrInstruct("if_entry" + ifn);
    _module->current_function()->addBlock("if_entry" + ifn);
    auto if_entry = _module->current_block();
    node.cond()->accept(*this);
    auto result = _module->current_block()->last()->reg();
    if (!Type::is_same(result->getType(), BoolType::get())) {
        auto tmp = result;
        result = std::make_shared<BoolValue>(_factory->next_reg());
        _factory->addTruncInstruct(result, tmp);
    }

    _module->current_function()->addBlock("if_body" + ifn);
    auto if_body = _module->current_block();

    std::shared_ptr<Block> else_body;
    if (node.else_stmt()) {
        _module->current_function()->addBlock("else_body" + ifn);
        else_body = _module->current_block();
    }

    _module->current_function()->addBlock("if_end" + ifn);
    auto if_end = _module->current_block(); 

    _module->current_function()->setBlock(if_entry);
    if (node.else_stmt()) {
        _factory->addCondBrInstruct(result, if_body->label(), else_body->label());
    } else {
        _factory->addCondBrInstruct(result, if_body->label(), if_end->label());
    }

    _module->current_function()->setBlock(if_body);
    node.if_stmt()->accept(*this);
    _factory->addBrInstruct("if_end" + ifn);

    if (node.else_stmt()) {
        _module->current_function()->setBlock(else_body);
        node.else_stmt()->accept(*this);
        _factory->addBrInstruct("else_body" + ifn);
    }

    _module->current_function()->setBlock(if_end);
}

void IrGenerator::visit(BlockStmtNode& node) {
    auto tmp = _current_table;
    _current_table = std::make_shared<BlockSymbolTable>(_current_table);

    node.block()->accept(*this);
    
    _current_table = tmp;
}

void IrGenerator::visit(RValNode& node) {
    switch (node.type()) {
        case RVAL_GETINT: {
            auto result = std::make_shared<IntValue>(_factory->next_reg());
            _factory->addCallExternalInstruct(result, "getint", {});
        }
        break;
        case RVAL_GETCHAR: {
            std::shared_ptr<Value> result = std::make_shared<IntValue>(_factory->next_reg());
            _factory->addCallExternalInstruct(result, "getchar", {});
        }
        break;
        case RVAL_EXP: {
            node.exp()->accept(*this);
        }
        break;
    }
}

void IrGenerator::visit(FuncRParamsNode& node) {}

void IrGenerator::visit(FuncFParamsNode& node) {}


}
}