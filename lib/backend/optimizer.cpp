#include "optimizer.hpp"
#include "ir.hpp"
#include <memory>

using namespace blang::entities;

namespace blang {
namespace backend {

Optimizer::Optimizer() : _passes({
    std::make_shared<EmptyBlockPass>()
}) {}

std::shared_ptr<IrModule> Optimizer::optim(std::shared_ptr<IrModule> module) {
    _module = module;
    for (auto& pass : _passes) {
        module = pass->optim(module);
    }
    return module;
}

void EmptyBlockPass::resetPreBlock(std::shared_ptr<Function> function, std::string from, std::string to) {
    for (auto& block : function->blocks()) {
        for (auto& label : block->next()) {
            if (label == from) {
                for (auto iter = block->instructions().begin(); iter < block->instructions().end(); iter++) {
                    if (auto br = std::dynamic_pointer_cast<BrInstruct>(*iter)) {
                        if (br->label() == from) {
                            auto next = block->instructions().erase(iter);
                            block->instructions().insert(next, std::make_shared<BrInstruct>(to));
                            iter = next - 1;
                        }
                    } else if (auto condbr = std::dynamic_pointer_cast<CondBrInstruct>(*iter)) {
                        auto true_label = condbr->true_label();
                        auto false_label = condbr->false_label();
                        bool flag = false;
                        if (true_label == from) {
                            true_label = to;
                            flag = true;
                        }
                        if (false_label == from) {
                            false_label = to;
                            flag = true;
                        }
                        if (flag) {
                            auto next = block->instructions().erase(iter);
                            block->instructions().insert(next, std::make_shared<CondBrInstruct>(condbr->cond(), true_label, false_label));
                            iter = next - 1;
                        }
                    }
                }
                break;
            }
        }
    }
}

std::shared_ptr<IrModule> EmptyBlockPass::optim(std::shared_ptr<IrModule> module) {
    for (auto& [ident, function] : module->functions()) {
        for (auto iter = function->blocks().begin(); iter < function->blocks().end(); iter++) {
            auto block = *iter;
            if (block->instructions().size() == 1) {
                if (auto br = std::dynamic_pointer_cast<BrInstruct>(
                    block->instructions().at(0)
                ); br) {
                    resetPreBlock(function, block->label(), br->label());
                    auto next = function->blocks().erase(iter);
                    iter = next - 1;
                }
            }
        }
    }

    return module;
}

}
}