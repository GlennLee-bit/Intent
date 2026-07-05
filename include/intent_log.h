#ifndef INTENT_LOG_H
#define INTENT_LOG_H

#include <stdio.h>

typedef enum {
    INTENT_LOG_LEVEL_ERROR = 0,
    INTENT_LOG_LEVEL_WARN = 1,
    INTENT_LOG_LEVEL_INFO = 2,
    INTENT_LOG_LEVEL_DEBUG = 3,
} IntentLogLevel;

void IntentLogSetLevel(IntentLogLevel level);
IntentLogLevel IntentLogGetLevel(void);

int IntentLogPrint(const char *level, const char *file, int line, const char *fmt, ...);

#define LOG_ERROR(fmt, ...) \
    IntentLogPrint("ERROR", __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) \
    IntentLogPrint("WARN", __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) \
    IntentLogPrint("INFO", __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) \
    IntentLogPrint("DEBUG", __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif /* INTENT_LOG_H */