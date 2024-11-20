#ifndef BLANG_OPTIMIZER_H
#define BLANG_OPTIMIZER_H

#include "ir.hpp"
#include <memory>
#include <string>
#include <vector>

using namespace blang::entities;

namespace blang {
namespace backend {

class Pass {
public:
    Pass() {}
    virtual ~Pass() {}
    virtual std::shared_ptr<IrModule> optim(std::shared_ptr<IrModule> module) = 0;
};

class Optimizer {
private:
    std::shared_ptr<IrModule> _module;  
    std::vector<std::shared_ptr<Pass>> _passes;
public:
    Optimizer();
    std::shared_ptr<IrModule> optim(std::shared_ptr<IrModule> module);
};

class EmptyBlockPass : public Pass {
    // TODO: optimize
private:
    void resetPreBlock(std::shared_ptr<Function> function, std::string from, std::string to);
public:
    EmptyBlockPass() = default;
    virtual ~EmptyBlockPass() = default;
    virtual std::shared_ptr<IrModule> optim(std::shared_ptr<IrModule> module) override;
};

}
}

#endif