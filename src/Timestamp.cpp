#include "Timestamp.h"

#include <time.h>

Timestamp::Timestamp() :m_microSecondsSinceEpoch(0) {}
Timestamp::Timestamp(int64_t microSecondsSinceEpoch) :m_microSecondsSinceEpoch(microSecondsSinceEpoch) {}
Timestamp Timestamp::now() {
    return Timestamp(time(NULL));
}
std::string Timestamp::toString() const {
    char buf[128] = {0};
    tm *tmTime = localtime(&m_microSecondsSinceEpoch);
    snprintf(buf,128,"%4d-%02d-%02d %02d:%02d:%02d",
    tmTime->tm_year + 1900,
    tmTime->tm_mon + 1,
    tmTime->tm_mday,
    tmTime->tm_hour,
    tmTime->tm_min,
    tmTime->tm_sec);

    return buf;
}
