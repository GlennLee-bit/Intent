#include "intent_log.h"
#include <stdarg.h>

/* 全局日志级别，运行时可调 */
static IntentLogLevel gLogLevel = INTENT_LOG_LEVEL_INFO;

/* 设置日志级别，运行时可调 */
void IntentLogSetLevel(IntentLogLevel level) {
    gLogLevel = level;
}

/* 获取当前日志级别 */
IntentLogLevel IntentLogGetLevel(void) {
    return gLogLevel;
}

/* 通用日志打印入口（被宏调用） */
int IntentLogPrint(const char *level, const char *file, int line,
                   const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int ret = fprintf(stderr, "[%s] %s:%d: ", level, file, line);
    ret += vfprintf(stderr, fmt, args);
    ret += fprintf(stderr, "\n");
    va_end(args);
    return ret;
}