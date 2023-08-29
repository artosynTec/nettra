#pragma once

#include <string>
#include <iostream>

class Timestamp {
private:
    int64_t m_microSecondsSinceEpoch;
public:
    Timestamp();
    explicit Timestamp(int64_t microSecondsSinceEpoch);
    static Timestamp now();
    std::string toString() const;
};
