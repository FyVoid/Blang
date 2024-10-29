#include "logger.hpp"
#include <algorithm>
#include <memory>
#include <vector>

namespace blang {

Logger::Logger() {
}

void Logger::logError(std::shared_ptr<ErrorLog> error) {
    _errors.push_back(error);
}

void Logger::log(std::shared_ptr<Log> log) {
    _logs.push_back(log);
}

void Logger::sortError() {
    std::stable_sort(_errors.begin(), errors().end(), 
        [](std::shared_ptr<ErrorLog> e1, std::shared_ptr<ErrorLog> e2) {
            return e1->line < e2->line;
        }
    );
}

std::vector<std::shared_ptr<SyntaxLog>> Logger::syntax_logs() {
    auto ret = std::vector<std::shared_ptr<SyntaxLog>>();

    for (auto& log : _logs) {
        if (auto slog = std::dynamic_pointer_cast<SyntaxLog>(log); slog) {
            ret.push_back(slog);
        }
    }

    std::stable_sort(ret.begin(), ret.end(),
        [](std::shared_ptr<SyntaxLog> l1, std::shared_ptr<SyntaxLog> l2) {
            return l1->blockn < l2->blockn;
        }
    );

    return ret;
}

}