#include "snapshot_parser.h"
#include "intent_log.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

/* 字符串复制辅助：复制 src 字符串到新分配的内存，失败返回 NULL */
static char *DuplicateString(const char *src) {
    if (src == NULL) return NULL;
    size_t len = strlen(src);
    char *dst = (char *)malloc(len + 1);
    if (dst == NULL) {
        LOG_ERROR("malloc failed for string duplication");
        return NULL;
    }
    memcpy(dst, src, len + 1);
    return dst;
}

/* 时间段枚举字符串 -> 枚举值映射 */
static int ParseTimePeriod(const char *str, PublicTimePeriod *out) {
    if (strcmp(str, "MIDNIGHT") == 0) *out = PUBLIC_TIME_PERIOD_MIDNIGHT;
    else if (strcmp(str, "MORNING") == 0) *out = PUBLIC_TIME_PERIOD_MORNING;
    else if (strcmp(str, "FORENOON") == 0) *out = PUBLIC_TIME_PERIOD_FORENOON;
    else if (strcmp(str, "NOON") == 0) *out = PUBLIC_TIME_PERIOD_NOON;
    else if (strcmp(str, "AFTERNOON") == 0) *out = PUBLIC_TIME_PERIOD_AFTERNOON;
    else if (strcmp(str, "NIGHT") == 0) *out = PUBLIC_TIME_PERIOD_NIGHT;
    else {
        LOG_ERROR("Unknown publicTimePeriod: %s", str);
        return INTENT_FILTER_ERR_PARSE;
    }
    return INTENT_FILTER_OK;
}

/* 工作日类型字符串 -> 枚举值映射 */
static int ParseWorkDay(const char *str, PublicWorkDay *out) {
    if (strcmp(str, "PUBLIC_HOLIDAY") == 0) *out = PUBLIC_WORK_DAY_HOLIDAY;
    else if (strcmp(str, "PUBLIC_WORKDAY") == 0) *out = PUBLIC_WORK_DAY_WORKDAY;
    else {
        LOG_ERROR("Unknown publicWorkDay: %s", str);
        return INTENT_FILTER_ERR_PARSE;
    }
    return INTENT_FILTER_OK;
}

/* 通勤时段字符串 -> 枚举值映射 */
static int ParseCommutePeriod(const char *str, CommutePeriod *out) {
    if (strcmp(str, "GO_TO_WORK") == 0) *out = COMMUTE_PERIOD_GO_TO_WORK;
    else if (strcmp(str, "LEAVE_COMPANY") == 0) *out = COMMUTE_PERIOD_LEAVE_COMPANY;
    else if (strcmp(str, "ARRIVE_HOME") == 0) *out = COMMUTE_PERIOD_ARRIVE_HOME;
    else if (strcmp(str, "LEAVE_HOME") == 0) *out = COMMUTE_PERIOD_LEAVE_HOME;
    else if (strcmp(str, "ARRIVE_COMPANY") == 0) *out = COMMUTE_PERIOD_ARRIVE_COMPANY;
    else {
        LOG_ERROR("Unknown commutePeriod: %s", str);
        return INTENT_FILTER_ERR_PARSE;
    }
    return INTENT_FILTER_OK;
}

/* 解析 POI 列表（poiType + startTime + endTime） */
static int ParsePoiList(const cJSON *arr, PoiInfo **outList, int *outCount) {
    int count = cJSON_GetArraySize(arr);
    if (count == 0) {
        *outList = NULL;
        *outCount = 0;
        return INTENT_FILTER_OK;
    }
    PoiInfo *list = (PoiInfo *)calloc(count, sizeof(PoiInfo));
    if (list == NULL) {
        LOG_ERROR("malloc failed for poi list, count=%d", count);
        return INTENT_FILTER_ERR_MEMORY;
    }
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        cJSON *typeNode = cJSON_GetObjectItem(item, "poiType");
        cJSON *startNode = cJSON_GetObjectItem(item, "startTime");
        cJSON *endNode = cJSON_GetObjectItem(item, "endTime");
        list[i].poiType = DuplicateString(cJSON_GetStringValue(typeNode));
        list[i].startTime = (long long)startNode->valuedouble;
        list[i].endTime = (long long)endNode->valuedouble;
    }
    *outList = list;
    *outCount = count;
    return INTENT_FILTER_OK;
}

/* 通用列表解析（timestamp + content + source）
 * elemSize 必须与具体结构大小匹配。 */
static int ParseTimeStampedList(const cJSON *arr, void *outList,
                                 int *outCount, size_t elemSize,
                                 const char *tsField,
                                 const char *contentField,
                                 const char *sourceField) {
    int count = cJSON_GetArraySize(arr);
    if (count == 0) {
        *(void **)outList = NULL;
        *outCount = 0;
        return INTENT_FILTER_OK;
    }
    char *base = (char *)calloc(count, elemSize);
    if (base == NULL) {
        LOG_ERROR("malloc failed for time stamped list, count=%d", count);
        return INTENT_FILTER_ERR_MEMORY;
    }
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        char *dst = base + i * elemSize;
        cJSON *tsNode = cJSON_GetObjectItem(item, tsField);
        *(long long *)dst = (long long)tsNode->valuedouble;
        dst += sizeof(long long);
        if (contentField != NULL) {
            cJSON *contentNode = cJSON_GetObjectItem(item, contentField);
            *(char **)dst = DuplicateString(cJSON_GetStringValue(contentNode));
            dst += sizeof(char *);
        }
        if (sourceField != NULL) {
            cJSON *sourceNode = cJSON_GetObjectItem(item, sourceField);
            *(char **)dst = DuplicateString(cJSON_GetStringValue(sourceNode));
        }
    }
    *(void **)outList = base;
    *outCount = count;
    return INTENT_FILTER_OK;
}

/* 解析 APP 使用列表（appName + startTime + endTime） */
static int ParseAppUsageList(const cJSON *arr, AppUsage **outList,
                              int *outCount) {
    int count = cJSON_GetArraySize(arr);
    if (count == 0) { *outList = NULL; *outCount = 0; return INTENT_FILTER_OK; }
    AppUsage *list = (AppUsage *)calloc(count, sizeof(AppUsage));
    if (list == NULL) return INTENT_FILTER_ERR_MEMORY;
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        list[i].appName = DuplicateString(cJSON_GetStringValue(
            cJSON_GetObjectItem(item, "appName")));
        list[i].startTime = (long long)cJSON_GetObjectItem(item, "startTime")->valuedouble;
        list[i].endTime = (long long)cJSON_GetObjectItem(item, "endTime")->valuedouble;
    }
    *outList = list;
    *outCount = count;
    return INTENT_FILTER_OK;
}

/* 解析 Companion/Screen 记忆列表（entityId + caption + entity + timestamp） */
static int ParseMemoryList(const cJSON *arr, CompanionMemory **outList,
                            int *outCount, const char *entityIdField) {
    int count = cJSON_GetArraySize(arr);
    if (count == 0) { *outList = NULL; *outCount = 0; return INTENT_FILTER_OK; }
    CompanionMemory *list = (CompanionMemory *)calloc(count, sizeof(CompanionMemory));
    if (list == NULL) return INTENT_FILTER_ERR_MEMORY;
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        list[i].entityId = DuplicateString(cJSON_GetStringValue(
            cJSON_GetObjectItem(item, entityIdField)));
        list[i].caption = DuplicateString(cJSON_GetStringValue(
            cJSON_GetObjectItem(item, "caption")));
        list[i].entity = DuplicateString(cJSON_GetStringValue(
            cJSON_GetObjectItem(item, "entity")));
        list[i].timestamp = (long long)cJSON_GetObjectItem(item, "timestamp")->valuedouble;
    }
    *outList = list;
    *outCount = count;
    return INTENT_FILTER_OK;
}

/* 释放 Snapshot 内所有动态分配字段 */
void SnapshotDestroy(Snapshot *Snapshot) {
    if (Snapshot == NULL) return;
    for (int i = 0; i < Snapshot->poiTodayCount; i++) {
        free(Snapshot->poiTodayList[i].poiType);
    }
    free(Snapshot->poiTodayList);
    for (int i = 0; i < Snapshot->appUsageCount; i++) {
        free(Snapshot->appUsageList[i].appName);
    }
    free(Snapshot->appUsageList);
    for (int i = 0; i < Snapshot->notificationCount; i++) {
        free(Snapshot->notificationList[i].content);
        free(Snapshot->notificationList[i].source);
    }
    free(Snapshot->notificationList);
    for (int i = 0; i < Snapshot->pasteBoardCount; i++) {
        free(Snapshot->pasteBoardList[i].content);
        free(Snapshot->pasteBoardList[i].source);
    }
    free(Snapshot->pasteBoardList);
    free(Snapshot->companionSummary.content);
    for (int i = 0; i < Snapshot->companionMemoryCount; i++) {
        free(Snapshot->companionMemoryList[i].entityId);
        free(Snapshot->companionMemoryList[i].caption);
        free(Snapshot->companionMemoryList[i].entity);
    }
    free(Snapshot->companionMemoryList);
    for (int i = 0; i < Snapshot->screenMemoryCount; i++) {
        free(Snapshot->screenMemoryList[i].entityId);
        free(Snapshot->screenMemoryList[i].caption);
        free(Snapshot->screenMemoryList[i].entity);
    }
    free(Snapshot->screenMemoryList);
    free(Snapshot->userProfile.character.name);
    free(Snapshot->userProfile.userMd);
}

/* 解析 JSON 字符串为 Snapshot 结构体 */
int SnapshotParseFromJson(const char *JsonString, Snapshot *Snapshot) {
    if (JsonString == NULL || Snapshot == NULL) {
        LOG_ERROR("Invalid params for SnapshotParseFromJson");
        return INTENT_FILTER_ERR_PARAM;
    }
    memset(Snapshot, 0, sizeof(Snapshot));
    cJSON *root = cJSON_Parse(JsonString);
    if (root == NULL) {
        LOG_ERROR("Failed to parse JSON root");
        return INTENT_FILTER_ERR_PARSE;
    }
    cJSON *features = cJSON_GetObjectItem(root, "features");
    cJSON *timeNode = cJSON_GetObjectItem(root, "snapshotTime");
    if (features == NULL || timeNode == NULL) {
        LOG_ERROR("Missing required fields: features or snapshotTime");
        cJSON_Delete(root);
        return INTENT_FILTER_ERR_PARSE;
    }
    Snapshot->snapshotTime = (long long)timeNode->valuedouble;
    int ret;
    ret = ParseTimePeriod(cJSON_GetStringValue(
        cJSON_GetObjectItem(features, "publicTimePeriod")),
        &Snapshot->publicTimePeriod);
    if (ret != INTENT_FILTER_OK) goto parseFail;
    ret = ParseWorkDay(cJSON_GetStringValue(
        cJSON_GetObjectItem(features, "publicWorkDay")),
        &Snapshot->publicWorkDay);
    if (ret != INTENT_FILTER_OK) goto parseFail;
    ret = ParseCommutePeriod(cJSON_GetStringValue(
        cJSON_GetObjectItem(features, "commutePeriod")),
        &Snapshot->commutePeriod);
    if (ret != INTENT_FILTER_OK) goto parseFail;
    cJSON *poiArr = cJSON_GetObjectItem(features, "poiToday");
    if (poiArr != NULL) {
        ret = ParsePoiList(poiArr, &Snapshot->poiTodayList, &Snapshot->poiTodayCount);
        if (ret != INTENT_FILTER_OK) goto parseFail;
    }
    cJSON *appArr = cJSON_GetObjectItem(features, "appUsage30min");
    if (appArr != NULL) {
        ret = ParseAppUsageList(appArr, &Snapshot->appUsageList, &Snapshot->appUsageCount);
        if (ret != INTENT_FILTER_OK) goto parseFail;
    }
    cJSON *notifArr = cJSON_GetObjectItem(features, "notifications10min");
    if (notifArr != NULL) {
        ret = ParseTimeStampedList(notifArr, &Snapshot->notificationList,
                                   &Snapshot->notificationCount,
                                   sizeof(Notification),
                                   "timestamp", "content", "source");
        if (ret != INTENT_FILTER_OK) goto parseFail;
    }
    cJSON *companionArr = cJSON_GetObjectItem(features, "latestCompanionMemoryInfo");
    if (companionArr != NULL) {
        ret = ParseMemoryList(companionArr, &Snapshot->companionMemoryList,
                              &Snapshot->companionMemoryCount, "entityId");
        if (ret != INTENT_FILTER_OK) goto parseFail;
    }
    cJSON *userProfile = cJSON_GetObjectItem(features, "userProfile");
    if (userProfile != NULL) {
        cJSON *character = cJSON_GetObjectItem(userProfile, "character");
        if (character != NULL) {
            cJSON *ageNode = cJSON_GetObjectItem(character, "age");
            cJSON *nameNode = cJSON_GetObjectItem(character, "name");
            if (ageNode != NULL) Snapshot->userProfile.character.age = ageNode->valueint;
            if (nameNode != NULL) Snapshot->userProfile.character.name =
                DuplicateString(cJSON_GetStringValue(nameNode));
        }
        cJSON *userMdNode = cJSON_GetObjectItem(userProfile, "user.md");
        if (userMdNode != NULL) {
            Snapshot->userProfile.userMd = DuplicateString(cJSON_GetStringValue(userMdNode));
        }
    }
    cJSON_Delete(root);
    return INTENT_FILTER_OK;
parseFail:
    cJSON_Delete(root);
    return ret;
}

/* 从文件加载并解析 Snapshot */
int SnapshotParseFromFile(const char *FilePath, Snapshot *Snapshot) {
    if (FilePath == NULL || Snapshot == NULL) return INTENT_FILTER_ERR_PARAM;
    FILE *fp = fopen(FilePath, "rb");
    if (fp == NULL) {
        LOG_ERROR("Failed to open file: %s", FilePath);
        return INTENT_FILTER_ERR_PARAM;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buf = (char *)malloc(size + 1);
    if (buf == NULL) {
        fclose(fp);
        LOG_ERROR("malloc failed for file buffer, size=%ld", size);
        return INTENT_FILTER_ERR_MEMORY;
    }
    size_t readSize = fread(buf, 1, size, fp);
    buf[readSize] = '\0';
    fclose(fp);
    int ret = SnapshotParseFromJson(buf, Snapshot);
    free(buf);
    return ret;
}