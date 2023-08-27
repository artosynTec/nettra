#pragma once

#include <string>

#define LOG_TRACE(logmsgformat, ...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(TRACE); \
        char buf[1024] = {0}; \
        snprintf(buf,1024,logmsgformat,##__VA_ARGS__); \
        logger.log(buf); \
    } while (0)

#define LOG_INFO(logmsgformat, ...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf,1024,logmsgformat,##__VA_ARGS__); \
        logger.log(buf); \
    } while (0)

#define LOG_WARN(logmsgformat, ...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(WARN); \
        char buf[1024] = {0}; \
        snprintf(buf,1024,logmsgformat,##__VA_ARGS__); \
        logger.log(buf); \
    } while (0)

#define LOG_ERROR(logmsgformat, ...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf,1024,logmsgformat,##__VA_ARGS__); \
        logger.log(buf); \
    } while (0)

#define LOG_FATAL(logmsgformat, ...) \
    do { \
        Logger &logger = Logger::instance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf,1024,logmsgformat,##__VA_ARGS__); \
        logger.log(buf); \
    } while (0)
    

// 日志级别定义
enum LogLevel {
    TRACE,
    INFO,
    WARN,
    ERROR,
    FATAL,
};

class Logger {
public:
    static Logger &instance();
    void setLogLevel(int logLevel);
    void log(std::string msg);
private:
    int m_logLevel;
};
