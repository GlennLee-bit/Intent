#include "scene_rule.h"
#include "intent_log.h"
#include <string.h>

/* 在 appSceneMap 中查找 appName 对应的条目索引，未找到返回 -1 */
static int FindAppInSceneMap(SceneConfig *Config, const char *AppName) {
    for (int i = 0; i < Config->appSceneMapCount; i++) {
        if (strcmp(Config->appSceneMap[i].appPackage, AppName) == 0) {
            return i;
        }
    }
    return -1;
}

/* 检查 fields 数组中是否包含 target 字段 */
static int HasField(FieldType *fields, int count, FieldType target) {
    for (int i = 0; i < count; i++) {
        if (fields[i] == target) return 1;
    }
    return 0;
}

/* 通过 APP 使用匹配场景：APP 在 appSceneMap 中映射到 Rule->sceneName */
static int MatchByAppUsage(Snapshot *Snapshot, SceneConfig *Config,
                            SceneRule *Rule) {
    for (int i = 0; i < Snapshot->appUsageCount; i++) {
        int idx = FindAppInSceneMap(Config, Snapshot->appUsageList[i].appName);
        if (idx >= 0 && strcmp(Config->appSceneMap[idx].sceneName,
                                Rule->sceneName) == 0) {
            return 1;
        }
    }
    return 0;
}

/* 通过 POI 上下文匹配场景（要求 Rule contextFields 包含 POI_TODAY 且 POI 列表非空） */
static int MatchByPoi(Snapshot *Snapshot, SceneRule *Rule) {
    if (!HasField(Rule->fieldView.contextFields,
                  Rule->fieldView.contextFieldCount, FIELD_POI_TODAY)) {
        return 0;
    }
    return Snapshot->poiTodayCount > 0 ? 1 : 0;
}

/* 通过通勤时段匹配场景 */
static int MatchByCommute(Snapshot *Snapshot, SceneRule *Rule) {
    if (!HasField(Rule->fieldView.contextFields,
                  Rule->fieldView.contextFieldCount, FIELD_COMMUTE_PERIOD)) {
        return 0;
    }
    if (strcmp(Rule->sceneName, "check_in") == 0) {
        return Snapshot->commutePeriod == COMMUTE_PERIOD_ARRIVE_COMPANY ? 1 : 0;
    }
    if (strcmp(Rule->sceneName, "navigation") == 0) {
        return Snapshot->commutePeriod == COMMUTE_PERIOD_GO_TO_WORK ||
               Snapshot->commutePeriod == COMMUTE_PERIOD_LEAVE_COMPANY ? 1 : 0;
    }
    return Snapshot->commutePeriod != COMMUTE_PERIOD_GO_TO_WORK ? 1 : 0;
}

/* 通过通知来源 APP 匹配场景 */
static int MatchByNotification(Snapshot *Snapshot, SceneConfig *Config,
                                SceneRule *Rule) {
    for (int i = 0; i < Snapshot->notificationCount; i++) {
        int idx = FindAppInSceneMap(Config, Snapshot->notificationList[i].source);
        if (idx >= 0 && strcmp(Config->appSceneMap[idx].sceneName,
                                Rule->sceneName) == 0) {
            return 1;
        }
    }
    return 0;
}

/* 主入口：枚举所有场景规则，命中任一维度即添加到结果 */
int MatchSceneRules(Snapshot *Snapshot, SceneConfig *Config,
                    SceneRule *MatchedRules, int *MatchedCount) {
    if (Snapshot == NULL || Config == NULL || MatchedRules == NULL
        || MatchedCount == NULL) {
        return INTENT_FILTER_ERR_PARAM;
    }
    *MatchedCount = 0;
    int matched = 0;
    for (int i = 0; i < Config->sceneRuleCount && matched < MAX_MATCHED_RULES; i++) {
        SceneRule *rule = &Config->sceneRules[i];
        if (MatchByAppUsage(Snapshot, Config, rule)
            || MatchByPoi(Snapshot, rule)
            || MatchByCommute(Snapshot, rule)
            || MatchByNotification(Snapshot, Config, rule)) {
            MatchedRules[matched++] = *rule;
        }
    }
    *MatchedCount = matched;
    return INTENT_FILTER_OK;
}

/* 根据规则和是否命中 APP 选择回溯策略 */
BacktrackStrategy DecideBacktrackStrategy(SceneRule *Rule, int HasAppMatch) {
    if (Rule == NULL) return BACKTRACK_STRATEGY_SELF_AS_INTENT;
    if (HasAppMatch) return BACKTRACK_STRATEGY_BY_APP_FIRST_ACTIVE;
    return BACKTRACK_STRATEGY_BY_STATE_CHANGE;
}