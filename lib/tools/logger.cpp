#include "logger.hpp"
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

}