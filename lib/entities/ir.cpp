#include "ir.hpp"
#include "type.hpp"
#include <memory>
#include <string>

namespace blang {
namespace entities {

std::string Block::to_string() {
    std::string ret = _label + ":\n";
    for (auto& instruct : _instructions) {
        ret += "    " + instruct->to_string() + "\n";
    }
    return ret;
}

void Function::addBlock(std::string label) {
    if (label.empty()) {
        label = _ident + "_label" + std::to_string(_blocks.size());
    }
    auto block = std::make_shared<Block>(label);
    _blocks.push_back(block);
    _current_block = block;
}

std::string Function::to_string() {
    std::string ret = "define ";
    ret += _ret_type->to_string() + " @" + _ident + "(";

    auto param_iter = _params.begin();
    if (!_params.empty()) {
        while ((param_iter + 1) != _params.end()) {
            auto [type, ident] = *param_iter;
            ret += type->to_string() + " %" + ident + ", ";
            param_iter++;
        }
        auto [type, ident] = *param_iter;
        ret += type->to_string() + " %" + ident;
    }
    ret += ") {\n";

    for (auto& block : _blocks) {
        ret += block->to_string();
    }

    ret += "}\n";
    return ret;
}

std::string IrModule::to_string() {
    std::string ret = std::string("declare i32 @getint()\n")
                        + "declare i32 @getchar()\n"
                        + "declare void @putint(i32)\n"
                        + "declare void @putchar(i32)\n"
                        + "declare void @putstr(i8*)\n";

    for (auto& instruct : _global) {
        ret += instruct->to_string() + "\n";
    }

    for (auto& [type, function] : _functions) {
        ret += function->to_string();
    }

    return ret;
}

void IrFactory::addDefInstruct(bool is_const, std::shared_ptr<PtrValue> var, std::shared_ptr<Value> init) {
    if (!_module->current_block()) {
        _module->global().push_back(std::make_shared<DefInstruct>(is_const, var, init));
    } else {
        _module->current_block()->push_back(std::make_shared<DefInstruct>(is_const, var, init));
    }
}

void IrFactory::addFunction(Type* ret_type, std::string ident, std::vector<std::tuple<Type*, std::string>> params) {
    auto function = std::make_shared<Function>(ret_type, ident, params);
    _module->functions().insert({ident, function});
    function->addBlock("entry_" + ident);
    _module->setFunction(function);
}

void IrFactory::addLoadInstruct(std::shared_ptr<Value> from, std::shared_ptr<Value> to) {
    _module->current_block()->push_back(std::make_shared<LoadInstruct>(from, to));
}

void IrFactory::addStoreInstruct(std::shared_ptr<Value> from, std::shared_ptr<PtrValue> to) {
    _module->current_block()->push_back(std::make_shared<StoreInstruct>(from, to));
}

void IrFactory::addAllocaInstruct(std::shared_ptr<PtrValue> var) {
    _module->current_block()->push_back(std::make_shared<AllocaInstruct>(var));
}

void IrFactory::addAddInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<AddInstruct>(reg, left, right));
}

void IrFactory::addSubInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<SubInstruct>(reg, left, right));
}

void IrFactory::addMulInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<MulInstruct>(reg, left, right));
}

void IrFactory::addDivInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<DivInstruct>(reg, left, right));
}

void IrFactory::addModInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<ModInstruct>(reg, left, right));
}

void IrFactory::addRetInstruct(std::shared_ptr<Value> ret_value) {
    _module->current_block()->push_back(std::make_shared<RetInstruct>(ret_value));
    _module->current_block()->ended() = true;
}

void IrFactory::addCallInstruct(std::shared_ptr<Value> result, std::shared_ptr<Function> function, std::vector<std::shared_ptr<Value>> params) {
    _module->current_block()->push_back(std::make_shared<CallInstruct>(result, function, params));
}

void IrFactory::addAndInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<AndInstruct>(reg, left, right));
}

void IrFactory::addOrInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<OrInstruct>(reg, left, right));
}

void IrFactory::addEqInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<EqInstruct>(reg, left, right));
}

void IrFactory::addNeqInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<NeqInstruct>(reg, left, right));
}

void IrFactory::addGeInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<GeInstruct>(reg, left, right));
}

void IrFactory::addGtInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<GtInstruct>(reg, left, right));
}

void IrFactory::addLtInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<LtInstruct>(reg, left, right));
}

void IrFactory::addLeInstruct(std::shared_ptr<Value> reg, std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    _module->current_block()->push_back(std::make_shared<LeInstruct>(reg, left, right));
}

void IrFactory::addGepInstruct(std::shared_ptr<PtrValue> result, std::shared_ptr<PtrValue> ptr, std::shared_ptr<IntConstValue> elem, std::shared_ptr<IntConstValue> offset) {
    _module->current_block()->push_back(std::make_shared<GEPInstruct>(result, ptr, elem, offset));
}

void IrFactory::addBrInstruct(std::string label) {
    _module->current_block()->next().push_back(label);
    _module->current_block()->push_back(std::make_shared<BrInstruct>(label));
    _module->current_block()->ended() = true;
}

void IrFactory::addCondBrInstruct(std::shared_ptr<Value> cond, std::string true_label, std::string false_label) {
    _module->current_block()->next().push_back(true_label);
    _module->current_block()->next().push_back(false_label);
    _module->current_block()->push_back(std::make_shared<CondBrInstruct>(cond, true_label, false_label));
    _module->current_block()->ended() = true;
}

void IrFactory::addSextInstruct(std::shared_ptr<Value> result, std::shared_ptr<Value> operand) {
    _module->current_block()->push_back(std::make_shared<SextInstruct>(result, operand));
}

void IrFactory::addZextInstruct(std::shared_ptr<Value> result, std::shared_ptr<Value> operand) {
    _module->current_block()->push_back(std::make_shared<ZextInstruct>(result, operand));
}

void IrFactory::addTruncInstruct(std::shared_ptr<Value> result, std::shared_ptr<Value> operand) {
    _module->current_block()->push_back(std::make_shared<TruncInstruct>(result, operand));
}

void IrFactory::addCallExternalInstruct(std::shared_ptr<Value> result, std::string function, std::vector<std::shared_ptr<Value>> params) {
    _module->current_block()->push_back(std::make_shared<CallExternalInstruct>(result, function, params));
}

}

}