#include "Logger.h"

#include <iostream>
#include <ctime>
#include <iomanip>

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

    std::time_t now =  std::time(nullptr);
    std::cout << " " << std::put_time(std::localtime(&now),"%F %T") << " " << msg << std::endl;
}