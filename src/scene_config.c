#include "scene_config.h"
#include "intent_log.h"
#include "cJSON.h"
#include <stdlib.h>
#include <string.h>

/* 字符串复制辅助：复制 src 字符串到新分配的内存 */
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

/* 解析字符串数组 */
static int ParseStringArray(const cJSON *arr, char ***outList, int *outCount) {
    int count = cJSON_GetArraySize(arr);
    if (count == 0) { *outList = NULL; *outCount = 0; return INTENT_FILTER_OK; }
    char **list = (char **)calloc(count, sizeof(char *));
    if (list == NULL) return INTENT_FILTER_ERR_MEMORY;
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        list[i] = DuplicateString(cJSON_GetStringValue(item));
    }
    *outList = list;
    *outCount = count;
    return INTENT_FILTER_OK;
}

/* 解析 APP -> Scene 映射对象（JSON 对象，key=package, value=sceneName） */
static int ParseAppSceneMap(const cJSON *obj, AppSceneMapEntry **outList,
                             int *outCount) {
    int count = cJSON_GetArraySize(obj);
    if (count == 0) { *outList = NULL; *outCount = 0; return INTENT_FILTER_OK; }
    AppSceneMapEntry *list = (AppSceneMapEntry *)calloc(count, sizeof(AppSceneMapEntry));
    if (list == NULL) return INTENT_FILTER_ERR_MEMORY;
    int idx = 0;
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, obj) {
        list[idx].appPackage = DuplicateString(item->string);
        list[idx].sceneName = DuplicateString(cJSON_GetStringValue(item));
        idx++;
    }
    *outList = list;
    *outCount = count;
    return INTENT_FILTER_OK;
}

/* FieldType 字符串 -> 枚举值映射 */
static int ParseFieldType(const char *str, FieldType *out) {
    if (strcmp(str, "snapshotTime") == 0) *out = FIELD_SNAPSHOT_TIME;
    else if (strcmp(str, "publicTimePeriod") == 0) *out = FIELD_PUBLIC_TIME_PERIOD;
    else if (strcmp(str, "publicWorkDay") == 0) *out = FIELD_PUBLIC_WORK_DAY;
    else if (strcmp(str, "commutePeriod") == 0) *out = FIELD_COMMUTE_PERIOD;
    else if (strcmp(str, "poiToday") == 0) *out = FIELD_POI_TODAY;
    else if (strcmp(str, "appUsage30min") == 0) *out = FIELD_APP_USAGE_30MIN;
    else if (strcmp(str, "notifications10min") == 0) *out = FIELD_NOTIFICATIONS_10MIN;
    else if (strcmp(str, "pasteBoard5min") == 0) *out = FIELD_PASTE_BOARD_5MIN;
    else if (strcmp(str, "latestCompanionMemoryInfo") == 0) *out = FIELD_LATEST_COMPANION_MEMORY_INFO;
    else if (strcmp(str, "latestScreenMemoryInfo") == 0) *out = FIELD_LATEST_SCREEN_MEMORY_INFO;
    else { LOG_ERROR("Unknown field: %s", str); return INTENT_FILTER_ERR_PARSE; }
    return INTENT_FILTER_OK;
}

/* 解析 FieldType 数组 */
static int ParseFieldArray(const cJSON *arr, FieldType **outList, int *outCount) {
    int count = cJSON_GetArraySize(arr);
    if (count == 0) { *outList = NULL; *outCount = 0; return INTENT_FILTER_OK; }
    FieldType *list = (FieldType *)calloc(count, sizeof(FieldType));
    if (list == NULL) return INTENT_FILTER_ERR_MEMORY;
    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(arr, i);
        int ret = ParseFieldType(cJSON_GetStringValue(item), &list[i]);
        if (ret != INTENT_FILTER_OK) {
            free(list);
            return ret;
        }
    }
    *outList = list;
    *outCount = count;
    return INTENT_FILTER_OK;
}

/* 解析 SceneFieldView（evidenceFields + stateFields + contextFields + 时间窗口） */
static int ParseFieldView(const cJSON *obj, SceneFieldView *outView) {
    cJSON *evArr = cJSON_GetObjectItem(obj, "evidenceFields");
    cJSON *stArr = cJSON_GetObjectItem(obj, "stateFields");
    cJSON *cxArr = cJSON_GetObjectItem(obj, "contextFields");
    cJSON *lbNode = cJSON_GetObjectItem(obj, "lookBackMs");
    cJSON *lfNode = cJSON_GetObjectItem(obj, "lookForwardMs");
    int ret;
    if ((ret = ParseFieldArray(evArr, &outView->evidenceFields,
                                &outView->evidenceFieldCount))
        != INTENT_FILTER_OK) return ret;
    if ((ret = ParseFieldArray(stArr, &outView->stateFields,
                                &outView->stateFieldCount))
        != INTENT_FILTER_OK) return ret;
    if ((ret = ParseFieldArray(cxArr, &outView->contextFields,
                                &outView->contextFieldCount))
        != INTENT_FILTER_OK) return ret;
    outView->lookBackMs = (long long)lbNode->valuedouble;
    outView->lookForwardMs = (long long)lfNode->valuedouble;
    return INTENT_FILTER_OK;
}

/* 释放 SceneConfig 内所有动态分配字段 */
void SceneConfigDestroy(SceneConfig *Config) {
    if (Config == NULL) return;
    for (int i = 0; i < Config->businessSceneCount; i++) {
        free(Config->businessScenes[i]);
    }
    free(Config->businessScenes);
    for (int i = 0; i < Config->appSceneMapCount; i++) {
        free(Config->appSceneMap[i].appPackage);
        free(Config->appSceneMap[i].sceneName);
    }
    free(Config->appSceneMap);
    for (int i = 0; i < Config->paymentAppCount; i++) {
        free(Config->paymentApps[i]);
    }
    free(Config->paymentApps);
    for (int i = 0; i < Config->sceneRuleCount; i++) {
        SceneRule *rule = &Config->sceneRules[i];
        free(rule->sceneName);
        free(rule->fieldView.evidenceFields);
        free(rule->fieldView.stateFields);
        free(rule->fieldView.contextFields);
    }
    free(Config->sceneRules);
}

/* 从 JSON 字符串加载 SceneConfig */
int SceneConfigLoadFromJson(const char *JsonString, SceneConfig *Config) {
    if (JsonString == NULL || Config == NULL) return INTENT_FILTER_ERR_PARAM;
    memset(Config, 0, sizeof(SceneConfig));
    cJSON *root = cJSON_Parse(JsonString);
    if (root == NULL) {
        LOG_ERROR("Failed to parse scene config JSON");
        return INTENT_FILTER_ERR_PARSE;
    }
    cJSON *bsArr = cJSON_GetObjectItem(root, "businessScenes");
    cJSON *appMap = cJSON_GetObjectItem(root, "appSceneMap");
    cJSON *payArr = cJSON_GetObjectItem(root, "paymentApps");
    cJSON *viewObj = cJSON_GetObjectItem(root, "sceneFieldViews");
    int ret;
    if ((ret = ParseStringArray(bsArr, &Config->businessScenes,
                                 &Config->businessSceneCount))
        != INTENT_FILTER_OK) goto fail;
    if ((ret = ParseAppSceneMap(appMap, &Config->appSceneMap,
                                 &Config->appSceneMapCount))
        != INTENT_FILTER_OK) goto fail;
    if ((ret = ParseStringArray(payArr, &Config->paymentApps,
                                 &Config->paymentAppCount))
        != INTENT_FILTER_OK) goto fail;
    int viewCount = cJSON_GetArraySize(viewObj);
    Config->sceneRules = (SceneRule *)calloc(viewCount, sizeof(SceneRule));
    if (Config->sceneRules == NULL) { ret = INTENT_FILTER_ERR_MEMORY; goto fail; }
    Config->sceneRuleCount = viewCount;
    int idx = 0;
    cJSON *viewItem = NULL;
    cJSON_ArrayForEach(viewItem, viewObj) {
        SceneRule *rule = &Config->sceneRules[idx];
        rule->sceneName = DuplicateString(viewItem->string);
        rule->strategy = BACKTRACK_STRATEGY_BY_APP_FIRST_ACTIVE;
        ret = ParseFieldView(viewItem, &rule->fieldView);
        if (ret != INTENT_FILTER_OK) goto fail;
        idx++;
    }
    cJSON_Delete(root);
    return INTENT_FILTER_OK;
fail:
    cJSON_Delete(root);
    SceneConfigDestroy(Config);
    return ret;
}

/* 从文件加载 SceneConfig */
int SceneConfigLoadFromFile(const char *FilePath, SceneConfig *Config) {
    if (FilePath == NULL || Config == NULL) return INTENT_FILTER_ERR_PARAM;
    FILE *fp = fopen(FilePath, "rb");
    if (fp == NULL) {
        LOG_ERROR("Failed to open scene config file: %s", FilePath);
        return INTENT_FILTER_ERR_PARAM;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buf = (char *)malloc(size + 1);
    if (buf == NULL) {
        fclose(fp);
        return INTENT_FILTER_ERR_MEMORY;
    }
    size_t readSize = fread(buf, 1, size, fp);
    buf[readSize] = '\0';
    fclose(fp);
    int ret = SceneConfigLoadFromJson(buf, Config);
    free(buf);
    return ret;
}