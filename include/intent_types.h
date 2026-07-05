#ifndef INTENT_TYPES_H
#define INTENT_TYPES_H

/* ===== 枚举定义（UPPER_SNAKE_CASE） ===== */

/* 时间段枚举 */
typedef enum {
    PUBLIC_TIME_PERIOD_MIDNIGHT = 0,
    PUBLIC_TIME_PERIOD_MORNING,
    PUBLIC_TIME_PERIOD_FORENOON,
    PUBLIC_TIME_PERIOD_NOON,
    PUBLIC_TIME_PERIOD_AFTERNOON,
    PUBLIC_TIME_PERIOD_NIGHT,
} PublicTimePeriod;

/* 工作日类型枚举 */
typedef enum {
    PUBLIC_WORK_DAY_HOLIDAY = 0,
    PUBLIC_WORK_DAY_WORKDAY,
} PublicWorkDay;

/* 通勤时段枚举 */
typedef enum {
    COMMUTE_PERIOD_GO_TO_WORK = 0,
    COMMUTE_PERIOD_LEAVE_COMPANY,
    COMMUTE_PERIOD_ARRIVE_HOME,
    COMMUTE_PERIOD_LEAVE_HOME,
    COMMUTE_PERIOD_ARRIVE_COMPANY,
} CommutePeriod;

/* 字段类型枚举 */
typedef enum {
    FIELD_SNAPSHOT_TIME = 0,
    FIELD_PUBLIC_TIME_PERIOD,
    FIELD_PUBLIC_WORK_DAY,
    FIELD_COMMUTE_PERIOD,
    FIELD_POI_TODAY,
    FIELD_APP_USAGE_30MIN,
    FIELD_NOTIFICATIONS_10MIN,
    FIELD_PASTE_BOARD_5MIN,
    FIELD_LATEST_COMPANION_MEMORY_INFO,
    FIELD_LATEST_SCREEN_MEMORY_INFO,
} FieldType;

/* 回溯策略枚举 */
typedef enum {
    BACKTRACK_STRATEGY_BY_APP_FIRST_ACTIVE = 0,
    BACKTRACK_STRATEGY_BY_APP_LAST_ACTIVE,
    BACKTRACK_STRATEGY_BY_STATE_CHANGE,
    BACKTRACK_STRATEGY_SELF_AS_INTENT,
} BacktrackStrategy;

/* 错误码 */
typedef enum {
    INTENT_FILTER_OK = 0,
    INTENT_FILTER_ERR_PARAM = -1,
    INTENT_FILTER_ERR_MEMORY = -2,
    INTENT_FILTER_ERR_PARSE = -3,
    INTENT_FILTER_ERR_NO_RULE = -4,
} IntentFilterErrorCode;

/* ===== 结构体定义 ===== */

typedef struct {
    char *poiType;
    long long startTime;
    long long endTime;
} PoiInfo;

typedef struct {
    char *appName;
    long long startTime;
    long long endTime;
} AppUsage;

typedef struct {
    long long timestamp;
    char *content;
    char *source;
} Notification;

typedef struct {
    long long timestamp;
    char *content;
    char *source;
} PasteBoard;

typedef struct {
    char *entityId;
    char *caption;
    char *entity;
    long long timestamp;
} CompanionMemory;

typedef struct {
    char *entityId;
    char *caption;
    char *entity;
    long long timestamp;
} ScreenMemory;

typedef struct {
    char *content;
} CompanionSummary;

typedef struct {
    int age;
    char *name;
} UserCharacter;

typedef struct {
    UserCharacter character;
    char *userMd;
} UserProfile;

typedef struct {
    long long snapshotTime;
    PublicTimePeriod publicTimePeriod;
    PublicWorkDay publicWorkDay;
    CommutePeriod commutePeriod;
    PoiInfo *poiTodayList;
    int poiTodayCount;
    AppUsage *appUsageList;
    int appUsageCount;
    Notification *notificationList;
    int notificationCount;
    PasteBoard *pasteBoardList;
    int pasteBoardCount;
    CompanionSummary companionSummary;
    CompanionMemory *companionMemoryList;
    int companionMemoryCount;
    ScreenMemory *screenMemoryList;
    int screenMemoryCount;
    UserProfile userProfile;
} Snapshot;

typedef struct {
    FieldType *evidenceFields;
    int evidenceFieldCount;
    FieldType *stateFields;
    int stateFieldCount;
    FieldType *contextFields;
    int contextFieldCount;
    long long lookBackMs;
    long long lookForwardMs;
} SceneFieldView;

typedef struct {
    char *sceneName;
    SceneFieldView fieldView;
    BacktrackStrategy strategy;
} SceneRule;

typedef struct {
    char *appPackage;
    char *sceneName;
} AppSceneMapEntry;

typedef struct {
    char **businessScenes;
    int businessSceneCount;
    AppSceneMapEntry *appSceneMap;
    int appSceneMapCount;
    char **paymentApps;
    int paymentAppCount;
    SceneRule *sceneRules;
    int sceneRuleCount;
} SceneConfig;

typedef struct {
    Snapshot *intentSnapshot;
    Snapshot *evidenceList;
    int evidenceCount;
    char *sceneName;
} IntentCandidate;

typedef struct {
    IntentCandidate *candidateList;
    int candidateCount;
} IntentResult;

#endif /* INTENT_TYPES_H */