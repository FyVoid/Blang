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
        auto ident = var->ident();
        if (Type::is_same(type, IntType::get())) {
            value = std::make_shared<IntConstValue>(*(var->get<int32_t>()));
            ptr = std::make_shared<PtrValue>(var->type(), true, ident);
        } else if (Type::is_same(type, CharType::get())) {
            value = std::make_shared<CharConstValue>(*(var->get<char>()));
            ptr = std::make_shared<PtrValue>(var->type(), true, ident);
        } else if (auto array_t = dynamic_cast<ArrayType*>(type); array_t) {
            if (Type::is_same(array_t->type(), IntType::get())) {
                std::vector<std::shared_ptr<Value>> vec{};
                auto array = var->get<int32_t>();
                for (size_t i = 0; i < array_t->length(); i++) {
                    vec.push_back(std::make_shared<IntConstValue>(array[i]));
                }
                value = std::make_shared<ArrayValue>(array_t, vec);
                ptr = std::make_shared<PtrValue>(var->type(), true, ident);
            } else if (Type::is_same(array_t->type(), CharType::get())) {
                std::vector<std::shared_ptr<Value>> vec{};
                auto array = var->get<char>();
                for (size_t i = 0; i < array_t->length(); i++) {
                    vec.push_back(std::make_shared<CharConstValue>(array[i]));
                }
                value = std::make_shared<ArrayValue>(array_t, vec);
                ptr = std::make_shared<PtrValue>(var->type(), true, ident);
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
        auto ident = _factory->next_reg();
        if (Type::is_same(type, IntType::get())) {
            ptr = std::make_shared<PtrValue>(var->type(), false, ident);
        } else if (Type::is_same(type, CharType::get())) {
            ptr = std::make_shared<PtrValue>(var->type(), false, ident);
        } else if (auto array_t = dynamic_cast<ArrayType*>(type); array_t) {
            if (Type::is_same(array_t->type(), IntType::get())) {
                std::vector<std::shared_ptr<Value>> vec{};
                auto array = var->get<int32_t>();
                for (size_t i = 0; i < array_t->length(); i++) {
                    vec.push_back(std::make_shared<IntConstValue>(array[i]));
                }
                ptr = std::make_shared<PtrValue>(var->type(), false, ident);
            } else if (Type::is_same(array_t->type(), CharType::get())) {
                std::vector<std::shared_ptr<Value>> vec{};
                auto array = var->get<char>();
                for (size_t i = 0; i < array_t->length(); i++) {
                    vec.push_back(std::make_shared<CharConstValue>(array[i]));
                }
                ptr = std::make_shared<PtrValue>(var->type(), false, ident);
            }
        }

        var->value() = ptr;
        _factory->addAllocaInstruct(ptr);

        if (def->init_val()) {
            if (Type::is_same(type, IntType::get())) {
                def->init_val()->exp()->accept(*this);
                auto result = _module->current_block()->last()->reg();
                if (Type::is_same(result->getType(), CharType::get())) {
                    auto tmp = std::make_shared<IntValue>(_factory->next_reg());
                    _factory->addSextInstruct(tmp, result);
                    result = tmp;
                }
                _factory->addStoreInstruct(result, ptr);
            } else if (Type::is_same(type, CharType::get())) {
                def->init_val()->exp()->accept(*this);
                auto result = _module->current_block()->last()->reg();
                if (Type::is_same(result->getType(), IntType::get())) {
                    auto reg = std::make_shared<CharValue>(_factory->next_reg());
                    _factory->addTruncInstruct(reg, result);
                    result = reg;
                }
                _factory->addStoreInstruct(result, ptr);
            } else if (auto array_t = dynamic_cast<ArrayType*>(type); array_t) {
                if (Type::is_same(array_t->type(), IntType::get())) {
                    int iter = 0;
                    for (auto& exp : def->init_val()->exps()) {
                        exp->accept(*this);
                        auto result = _module->current_block()->last()->reg();
                        if (Type::is_same(result->getType(), CharType::get())) {
                            auto tmp = std::make_shared<IntValue>(_factory->next_reg());
                            _factory->addSextInstruct(tmp, result);
                            result = tmp;
                        }
                        auto gep_ptr = std::make_shared<PtrValue>(array_t->type(), false, _factory->next_reg());
                        auto elem = std::make_shared<IntConstValue>(0);
                        auto offset = std::make_shared<IntConstValue>(iter);
                        iter++;
                        _factory->addGepInstruct(gep_ptr, ptr, elem, offset);
                        _factory->addStoreInstruct(result, gep_ptr);
                    }
                } else if (Type::is_same(array_t->type(), CharType::get())) {
                    if (def->init_val()->type() == entities::INIT_ARRAY) {
                        int iter = 0;
                        for (auto& exp : def->init_val()->exps()) {
                            exp->accept(*this);
                            auto result = _module->current_block()->last()->reg();
                            if (Type::is_same(result->getType(), IntType::get())) {
                                auto tmp = std::make_shared<CharValue>(_factory->next_reg());
                                _factory->addTruncInstruct(tmp, result);
                                result = tmp;
                            }
                            auto gep_ptr = std::make_shared<PtrValue>(array_t->type(), false, _factory->next_reg());
                            auto elem = std::make_shared<IntConstValue>(0);
                            auto offset = std::make_shared<IntConstValue>(iter);
                            iter++;
                            _factory->addGepInstruct(gep_ptr, ptr, elem, offset);
                            _factory->addStoreInstruct(result, gep_ptr);
                        }
                    } else if (def->init_val()->type() == entities::INIT_STRING) {
                        int iter = 0;
                        for (auto& ch : def->init_val()->str()) {
                            auto gep_ptr = std::make_shared<PtrValue>(array_t->type(), false, _factory->next_reg());
                            auto elem = std::make_shared<IntConstValue>(0);
                            auto offset = std::make_shared<IntConstValue>(iter);
                            iter++;
                            _factory->addGepInstruct(gep_ptr, ptr, elem, offset);
                            auto value = std::make_shared<CharConstValue>(ch);
                            _factory->addStoreInstruct(value, gep_ptr);
                        }
                    }
                }
            }
        }
    }
}

void IrGenerator::visit(DefNode& node) {}

void IrGenerator::visit(InitValNode& node) {}

void IrGenerator::visit(UnaryExpNode& node) {
    std::shared_ptr<Value> result;
    if (node.unary_exp()) {
        node.unary_exp()->accept(*this);
        switch (node.op()) {
            case entities::OP_NOT: {
                auto reg = _module->current_block()->last()->reg();
                if (!Type::is_same(reg->getType(), BoolType::get())) {
                    auto tmp = std::make_shared<BoolValue>(_factory->next_reg());
                    if (Type::is_same(reg->getType(), IntType::get())) {
                        _factory->addNeqInstruct(tmp, reg, std::make_shared<IntConstValue>(0));
                    } else if (Type::is_same(reg->getType(), CharType::get())) {
                        _factory->addNeqInstruct(tmp, reg, std::make_shared<CharConstValue>(0));
                    }
                    reg = tmp;
                }
                result = std::make_shared<BoolValue>(_factory->next_reg());
                _factory->addEqInstruct(result, reg, std::make_shared<BoolConstValue>(0));
            }
            break;
            case entities::OP_MINUS: {
                auto reg = _module->current_block()->last()->reg();
                std::shared_ptr<Value> from;
                if (Type::is_same(reg->getType(), IntType::get())) {
                    result = std::make_shared<IntValue>(_factory->next_reg());
                    from = std::make_shared<IntConstValue>(0);
                } else if (Type::is_same(reg->getType(), CharType::get())) {
                    result = std::make_shared<CharValue>(_factory->next_reg());
                    from = std::make_shared<CharConstValue>(0);
                } else if (Type::is_same(reg->getType(), BoolType::get())) {
                    result = std::make_shared<BoolValue>(_factory->next_reg());
                    from = std::make_shared<BoolConstValue>(0);
                }
                _factory->addSubInstruct(result, from, reg);
            }
            default:
            break;
        }
    } else if (node.primary_exp()) {
        node.primary_exp()->accept(*this);
        result = _module->current_block()->last()->reg();
    } else if (!node.ident().empty()) {
        auto func = _current_table->getFunc(node.ident());
        auto func_params = func->params();
        std::vector<std::shared_ptr<Value>> params{};
        if (node.func_rparams()) {
            int iter = 0;
            for (auto& param : node.func_rparams()->nodes()) {
                param->accept(*this);
                auto [type, ident] = func_params.at(iter);
                auto result = _module->current_block()->last()->reg();
                if (!Type::is_same(result->getType(), type)) {
                    if (Type::is_same(type, IntType::get())) {
                        auto tmp = std::make_shared<IntValue>(_factory->next_reg());
                        _factory->addSextInstruct(tmp, result);
                        result = tmp;
                    } else if (Type::is_same(type, CharType::get())) {
                        auto tmp = std::make_shared<CharValue>(_factory->next_reg());
                        _factory->addTruncInstruct(tmp, result);
                        result = tmp;
                    }
                }
                params.push_back(result);
                iter++;
            }
        }

        if (Type::is_same(func->return_type(), VoidType::get())) {
            result = nullptr;
        } else {
            if (Type::is_same(func->return_type(), IntType::get())) {
                result = std::make_shared<IntValue>(_factory->next_reg());
            } else if (Type::is_same(func->return_type(), CharType::get())) {
                result = std::make_shared<CharValue>(_factory->next_reg());
            }
        }

        _factory->addCallInstruct(result, func->function(), params);
    }
}

void IrGenerator::visit(BinaryExpNode& node) {
    auto result_bw = 0;
    switch (node.op()) {
        case entities::OP_ADD:
        case entities::OP_MINUS:
        case entities::OP_MUL:
        case entities::OP_MOD:
        case entities::OP_DIV:
        case entities::OP_EQ:
        case entities::OP_NEQ:
        case entities::OP_GE:
        case entities::OP_GT:
        case entities::OP_LE:
        case entities::OP_LT:
            result_bw = 32;
            break;
        case entities::OP_AND: {
            auto andn = _factory->next_block();
            _factory->addBrInstruct("and_entry" + andn);

            _module->current_function()->addBlock("and_entry" + andn);
            auto alloca = std::make_shared<PtrValue>(BoolType::get(), false, _factory->next_reg());
            _factory->addAllocaInstruct(alloca);
            _factory->addBrInstruct("and_left" + andn);

            _module->current_function()->addBlock("and_left" + andn);
            node.left()->accept(*this);
            auto reg = _module->current_block()->last()->reg();
            if (Type::is_same(reg->getType(), IntType::get())) {
                auto result = std::make_shared<BoolValue>(_factory->next_reg());
                _factory->addNeqInstruct(result, reg, std::make_shared<IntConstValue>(0));
                reg = result;
            } else if (Type::is_same(reg->getType(), CharType::get())) {
                auto result = std::make_shared<BoolValue>(_factory->next_reg());
                _factory->addNeqInstruct(result, reg, std::make_shared<CharConstValue>(0));
                reg = result;
            }
            _factory->addCondBrInstruct(reg, "and_right" + andn, "and_false" + andn);

            _module->current_function()->addBlock("and_right" + andn);
            node.right()->accept(*this);
            reg = _module->current_block()->last()->reg();
            if (Type::is_same(reg->getType(), IntType::get())) {
                auto result = std::make_shared<BoolValue>(_factory->next_reg());
                _factory->addNeqInstruct(result, reg, std::make_shared<IntConstValue>(0));
                reg = result;
            } else if (Type::is_same(reg->getType(), CharType::get())) {
                auto result = std::make_shared<BoolValue>(_factory->next_reg());
                _factory->addNeqInstruct(result, reg, std::make_shared<CharConstValue>(0));
                reg = result;
            }
            _factory->addCondBrInstruct(reg, "and_true" + andn, "and_false" + andn);

            _module->current_function()->addBlock("and_true" + andn);
            auto result = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addAddInstruct(result, std::make_shared<BoolConstValue>(1), std::make_shared<BoolConstValue>(0));
            _factory->addStoreInstruct(result, alloca);
            _factory->addBrInstruct("and_end" + andn);

            _module->current_function()->addBlock("and_false" + andn);
            result = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addAddInstruct(result, std::make_shared<BoolConstValue>(0), std::make_shared<BoolConstValue>(0));
            _factory->addStoreInstruct(result, alloca);
            _factory->addBrInstruct("and_end" + andn);

            _module->current_function()->addBlock("and_end" + andn);
            result = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addLoadInstruct(alloca, result);
            return ;
        }
        break;
        case entities::OP_OR: {
            auto orn = _factory->next_block();
            _factory->addBrInstruct("or_entry" + orn);

            _module->current_function()->addBlock("or_entry" + orn);
            auto alloca = std::make_shared<PtrValue>(BoolType::get(), false, _factory->next_reg());
            _factory->addAllocaInstruct(alloca);
            _factory->addBrInstruct("or_left" + orn);

            _module->current_function()->addBlock("or_left" + orn);
            node.left()->accept(*this);
            auto reg = _module->current_block()->last()->reg();
            if (Type::is_same(reg->getType(), IntType::get())) {
                auto result = std::make_shared<BoolValue>(_factory->next_reg());
                _factory->addNeqInstruct(result, reg, std::make_shared<IntConstValue>(0));
                reg = result;
            } else if (Type::is_same(reg->getType(), CharType::get())) {
                auto result = std::make_shared<BoolValue>(_factory->next_reg());
                _factory->addNeqInstruct(result, reg, std::make_shared<CharConstValue>(0));
                reg = result;
            }
            _factory->addCondBrInstruct(reg, "or_true" + orn, "or_right" + orn);

            _module->current_function()->addBlock("or_right" + orn);
            node.right()->accept(*this);
            reg = _module->current_block()->last()->reg();
            if (Type::is_same(reg->getType(), IntType::get())) {
                auto result = std::make_shared<BoolValue>(_factory->next_reg());
                _factory->addNeqInstruct(result, reg, std::make_shared<IntConstValue>(0));
                reg = result;
            } else if (Type::is_same(reg->getType(), CharType::get())) {
                auto result = std::make_shared<BoolValue>(_factory->next_reg());
                _factory->addNeqInstruct(result, reg, std::make_shared<CharConstValue>(0));
                reg = result;
            }
            _factory->addCondBrInstruct(reg, "or_true" + orn, "or_false" + orn);

            _module->current_function()->addBlock("or_true" + orn);
            auto result = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addAddInstruct(result, std::make_shared<BoolConstValue>(1), std::make_shared<BoolConstValue>(0));
            _factory->addStoreInstruct(result, alloca);
            _factory->addBrInstruct("or_end" + orn);

            _module->current_function()->addBlock("or_false" + orn);
            result = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addAddInstruct(result, std::make_shared<BoolConstValue>(0), std::make_shared<BoolConstValue>(0));
            _factory->addStoreInstruct(result, alloca);
            _factory->addBrInstruct("or_end" + orn);

            _module->current_function()->addBlock("or_end" + orn);
            result = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addLoadInstruct(alloca, result);
            return ;
        }
        break;
        default:
            break;
    }

    node.left()->accept(*this);
    auto left = _module->current_block()->last()->reg();
    if (result_bw == 32) {
        if (Type::is_same(left->getType(), CharType::get())) {
            auto reg = std::make_shared<IntValue>(_factory->next_reg());
            _factory->addSextInstruct(reg, left);
            left = _module->current_block()->last()->reg();
        } else if (Type::is_same(left->getType(), BoolType::get())) {
            auto reg = std::make_shared<IntValue>(_factory->next_reg());
            _factory->addZextInstruct(reg, left);
            left = _module->current_block()->last()->reg();
        }
    } else if (result_bw == 1) {
        if (Type::is_same(left->getType(), IntType::get())) {
            auto reg = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addNeqInstruct(reg, left, std::make_shared<IntConstValue>(0));
            left = _module->current_block()->last()->reg();
        } else if (Type::is_same(left->getType(), CharType::get())) {
            auto reg = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addNeqInstruct(reg, left, std::make_shared<CharConstValue>(0));
            left = _module->current_block()->last()->reg();
        }
    }

    node.right()->accept(*this);
    auto right = _module->current_block()->last()->reg();
    if (result_bw == 32) {
        if (Type::is_same(right->getType(), CharType::get())) {
            auto reg = std::make_shared<IntValue>(_factory->next_reg());
            _factory->addSextInstruct(reg, right);
            right = _module->current_block()->last()->reg();
        } else if (Type::is_same(right->getType(), BoolType::get())) {
            auto reg = std::make_shared<IntValue>(_factory->next_reg());
            _factory->addZextInstruct(reg, right);
            right = _module->current_block()->last()->reg();
        }
    } else if (result_bw == 1) {
        if (Type::is_same(right->getType(), IntType::get())) {
            auto reg = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addNeqInstruct(reg, right, std::make_shared<IntConstValue>(0));
            right = _module->current_block()->last()->reg();
        } else if (Type::is_same(right->getType(), CharType::get())) {
            auto reg = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addNeqInstruct(reg, right, std::make_shared<CharConstValue>(0));
            right = _module->current_block()->last()->reg();
        }
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

// load value of this node to a temp register
void IrGenerator::visit(LValNode& node) {
    auto var = _current_table->getVar(node.ident());
    Type* type;
    std::shared_ptr<Value> value;
    if (auto array_t = dynamic_cast<ArrayType*>(var->type()); array_t) {
        std::shared_ptr<Value> offset;
        if (node.exp()) {
            node.exp()->accept(*this);
            offset = _module->current_block()->last()->reg();
        } else {
            offset = std::make_shared<IntConstValue>(0);
        }
        auto result = std::make_shared<PtrValue>(array_t->type(), false, _factory->next_reg());
        auto elem = std::make_shared<IntConstValue>(0);
        _factory->addGepInstruct(result, var->value(), elem, std::static_pointer_cast<IntConstValue>(offset));
        if (!node.exp()) {
            return ;
        }
        type = array_t->type();
        value = result;
    } else if (auto ptr_t = dynamic_cast<PtrType*>(var->type()); ptr_t) {
        std::shared_ptr<Value> offset;
        if (node.exp()) {
            node.exp()->accept(*this);
            offset = _module->current_block()->last()->reg();
        } else {
            offset = std::make_shared<IntConstValue>(0);
        }
        auto result = std::make_shared<PtrValue>(ptr_t->next(), false, _factory->next_reg());
        _factory->addGepInstruct(result, var->value(), nullptr, std::static_pointer_cast<IntConstValue>(offset));
        if (!node.exp()) {
            return ;
        }
        type = ptr_t->next();
        value = result;
    } else {
        type = var->type();
        value = var->value();
    }

    if (Type::is_same(type, IntType::get())) {
        _factory->addLoadInstruct(value, std::make_shared<IntValue>(_factory->next_reg()));
    } else if (Type::is_same(type, CharType::get())) {
        _factory->addLoadInstruct(value, std::make_shared<CharValue>(_factory->next_reg()));
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
    if (node.params()) {
        node.params()->accept(*this);
    }
    node.block()->accept(*this);
    if (Type::is_same(node.type(), VoidType::get())) {
        _factory->addRetInstruct(nullptr);
    }
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
    if (_module->current_block()->ended()) {
        return ;
    }
    if (node.decl()) {
        node.decl()->accept(*this);
    } else if (node.stmt()) {
        node.stmt()->accept(*this);
    }
}

void IrGenerator::visit(AssignStmtNode& node) {
    auto lval = node.lval();
    auto var = _current_table->getVar(lval->ident());
    Type* type;
    std::shared_ptr<Value> ptr;
    if (auto array_t = dynamic_cast<ArrayType*>(var->type()); array_t) {
        lval->exp()->accept(*this);
        auto offset = _module->current_block()->last()->reg();
        auto result = std::make_shared<PtrValue>(array_t->type(), false, _factory->next_reg());
        auto elem = std::make_shared<IntConstValue>(0);
        _factory->addGepInstruct(result, var->value(), elem, std::static_pointer_cast<IntConstValue>(offset));
        type = array_t->type();
        ptr = result;
    } else if (auto ptr_t = dynamic_cast<PtrType*>(var->type()); ptr_t) {
        lval->exp()->accept(*this);
        auto offset = _module->current_block()->last()->reg();
        auto result = std::make_shared<PtrValue>(ptr_t->next(), false, _factory->next_reg());
        _factory->addGepInstruct(result, var->value(), nullptr, std::static_pointer_cast<IntConstValue>(offset));
        type = ptr_t->next();
        ptr = result;
    } else {
        type = var->type();
        ptr = var->value();
    }

    node.rval()->accept(*this);
    auto value = _module->current_block()->last()->reg();

    if (!Type::is_same(type, value->getType())) {
        if (Type::is_same(type, CharType::get())) {
            auto result = std::make_shared<CharValue>(_factory->next_reg());
            _factory->addTruncInstruct(result, value);
            value = result;
        } else if (Type::is_same(type, IntType::get())) {
            auto result = std::make_shared<IntValue>(_factory->next_reg());
            _factory->addSextInstruct(result, value);
            value = result;
        }
    }

    _factory->addStoreInstruct(value, std::dynamic_pointer_cast<PtrValue>(ptr));
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
            auto alloca = std::make_shared<PtrValue>(type, false, _factory->next_reg());
            _factory->addAllocaInstruct(alloca);
            _factory->addStoreInstruct(str_array, alloca);
            auto ptr = std::make_shared<PtrValue>(CharType::get(), false, _factory->next_reg());
            _factory->addGepInstruct(ptr, alloca, std::make_shared<IntConstValue>(0), std::make_shared<IntConstValue>(0));
            _factory->addCallExternalInstruct(nullptr, "putstr", {ptr});
        }
        if (d_next < c_next && d_next != std::string::npos) {
            params[param_count++]->accept(*this);
            auto reg = _module->current_block()->last()->reg();
            if (!Type::is_same(reg->getType(), IntType::get())) {
                auto result = std::make_shared<IntValue>(_factory->next_reg());
                _factory->addSextInstruct(result, reg);
                reg = result;
            }
            _factory->addCallExternalInstruct(nullptr, "putint", {reg});
            pos = next + 2;
        } else if (c_next < d_next && c_next != std::string::npos) {
            params[param_count++]->accept(*this);
            auto reg = _module->current_block()->last()->reg();
            if (!Type::is_same(reg->getType(), IntType::get())) {
                auto result = std::make_shared<IntValue>(_factory->next_reg());
                _factory->addSextInstruct(result, reg);
                reg = result;
            }
            _factory->addCallExternalInstruct(nullptr, "putchar", {reg});
            pos = next + 2;
        }
    }
}

void IrGenerator::visit(BreakStmtNode& node) {
    _factory->addBrInstruct(_for_end_labels.at(_for_end_labels.size() - 1));
}

void IrGenerator::visit(ContinueStmtNode& node) {
    _factory->addBrInstruct(_for_out_labels.at(_for_out_labels.size() - 1));
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
    _for_out_labels.push_back("for_out" + forn);
    _for_end_labels.push_back("for_end" + forn);

    _module->current_function()->addBlock("for_in" + forn);
    auto for_in = _module->current_function()->current_block();
    if (node.cond()) {
        node.cond()->accept(*this);
        auto result = _module->current_block()->last()->reg();
        if (!Type::is_same(result->getType(), BoolType::get())) {
            std::shared_ptr<Value> value;
            if (Type::is_same(result->getType(), CharType::get())) {
                value = std::make_shared<CharConstValue>(0);
            } else if (Type::is_same(result->getType(), IntType::get())) {
                value = std::make_shared<IntConstValue>(0);
            }
            auto tmp = result;
            result = std::make_shared<BoolValue>(_factory->next_reg());
            _factory->addNeqInstruct(result, tmp, value);
        }
        _factory->addCondBrInstruct(result, "for_body" + forn, "for_end" + forn);
    } else {
        _factory->addBrInstruct("for_body" + forn);
    }

    _module->current_function()->addBlock("for_body" + forn);
    auto for_body = _module->current_function()->current_block();
    node.stmt()->accept(*this);
    _factory->addBrInstruct("for_out" + forn);

    _module->current_function()->addBlock("for_out" + forn);
    auto for_out = _module->current_function()->current_block();
    if (node.for_out()) {
        node.for_out()->accept(*this);
    }
    _factory->addBrInstruct("for_in" + forn);

    _for_out_labels.pop_back();
    _for_end_labels.pop_back();

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
        _factory->addNeqInstruct(result, tmp, std::make_shared<IntConstValue>(0));
    }

    if (node.else_stmt()) {
        _factory->addCondBrInstruct(result, "if_body" + ifn, "else_body" + ifn);
    } else {
        _factory->addCondBrInstruct(result, "if_body" + ifn, "if_end" + ifn);
    }

    _module->current_function()->addBlock("if_body" + ifn);
    node.if_stmt()->accept(*this);
    _factory->addBrInstruct("if_end" + ifn);

    if (node.else_stmt()) {
        _module->current_function()->addBlock("else_body" + ifn);
        node.else_stmt()->accept(*this);
        _factory->addBrInstruct("if_end" + ifn);
    }

    _module->current_function()->addBlock("if_end" + ifn);
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

void IrGenerator::visit(FuncFParamsNode& node) {
    for (auto& [type, ident] : node.params()) {
        std::shared_ptr<Var> var;
        std::string log_type = "";
        std::shared_ptr<Value> value;
        if (auto ptr_t = dynamic_cast<PtrType*>(type); ptr_t) {
            auto base_t = ptr_t->next();
            if (Type::is_same(base_t, IntType::get())) {
                var = Var::getIntPtr(ident, false);
                var->value() = std::make_shared<PtrValue>(base_t, false, ident);
            } else if (Type::is_same(base_t, CharType::get())) {
                var = Var::getCharPtr(ident, false);
                var->value() = std::make_shared<PtrValue>(base_t, false, ident);
            }
        } else {
            if (Type::is_same(type, IntType::get())) {
                var = Var::getInt(ident, false);
                value = std::make_shared<IntValue>(ident);
                var->value() = std::make_shared<PtrValue>(IntType::get(), false, _factory->next_reg());
                _factory->addAllocaInstruct(var->value());
                _factory->addStoreInstruct(value, var->value());
            } else if (Type::is_same(type, CharType::get())) {
                var = Var::getChar(ident, false);
                value = std::make_shared<CharValue>(ident);
                var->value() = std::make_shared<PtrValue>(CharType::get(), false, _factory->next_reg());
                _factory->addAllocaInstruct(var->value());
                _factory->addStoreInstruct(value, var->value());
            }
        }
        _current_table->addVar(var);
    }
}


}
}