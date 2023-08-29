#include "Logger.h"
#include "Timestamp.h"

#include <iostream>

Logger &Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLogLevel(int logLevel) {
    m_logLevel = logLevel;
}

void Logger::log(std::string msg) {
    switch (m_logLevel) {
    case TRACE:
        std::cout << "[TRACE]";
        break;
    case INFO:
        std::cout << "[INFO]";
        break;
    case WARN:
        std::cout << "[WARN]";
        break;
    case ERROR:
        std::cout << "[ERROR]";
        break;
    case FATAL:
        std::cout << "[FATAL]";
        break;
    default:
        break;
    }

    std::cout << " " << Timestamp::now().toString() << " | " << msg << std::endl;
}