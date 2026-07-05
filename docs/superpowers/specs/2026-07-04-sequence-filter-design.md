# 序列筛选与特征关联 — 设计文档

> 日期: 2026-07-04
> 状态: 待审核

## 1. 概述

**项目目标**: 在嵌入式/移动端环境下，实现一个实时/近实时的意图识别服务。输入一批手机快照（snapshot），通过场景规则匹配找到候选场景，回溯查找意图发生的时刻，收集相关证据，最终输出 `{snapshot: 意图快照, evidence: [证据快照列表]}` 结构。

**核心约束**:
- 运行环境: 嵌入式/移动端
- 语言: C
- 数据来源: 其他模块推送 + 本地回溯
- 输出方式: 函数接口参数返回
- 场景规则: 初始化时从JSON配置加载一次，运行时不可变（5个场景: food_ordering, coffee_ordering, navigation, check_in, grocery_purchase）

## 2. 编码规范

1. 符合C语言编码规范: 函数使用大驼峰格式（如 `IntentFilterProcess`），变量名称使用首字母小写不带下划线（如 `snapshotCount`），参数类型定义使用首字母大写不带下划线（如 `Snapshot`、`IntentResult`）
2. 枚举值使用全大写带下划线（UPPER_SNAKE_CASE）格式（如 `PUBLIC_TIME_PERIOD_NOON`、`BACKTRACK_STRATEGY_BY_APP_FIRST_ACTIVE`）
3. 每个函数不超过50行
4. 申请释放内存保证规范，特别是异常分支处理
5. 异常分支需要有日志打印
6. 尽可能有详细的功能注释
7. 对于JSON数据的解析使用标准的函数（cJSON）

## 3. 架构方案: 表驱动规则引擎

选择表驱动规则引擎架构，理由:
- 场景规则以静态表形式定义，新增场景只需在表中加一行，不改核心逻辑
- 规则表可放在单独文件，维护分离
- 适合嵌入式环境（静态表是编译期常量，无动态加载开销）
- 函数指针在嵌入式C中是标准做法

结合环形缓冲区思路处理回溯窗口: 维护快照数组的索引范围作为回溯窗口，按时间范围（lookBackMs/lookForwardMs）筛选。

## 4. 数据模型

### 4.1 输入: Snapshot

上游以用户和时间范围为单位提供，每个快照包含 snapshotTime 和 features 字段。

内部结构体定义（从JSON解析后）:

```c
typedef enum {
    PUBLIC_TIME_PERIOD_MIDNIGHT,
    PUBLIC_TIME_PERIOD_MORNING,
    PUBLIC_TIME_PERIOD_FORENOON,
    PUBLIC_TIME_PERIOD_NOON,
    PUBLIC_TIME_PERIOD_AFTERNOON,
    PUBLIC_TIME_PERIOD_NIGHT,
} PublicTimePeriod;

typedef enum {
    PUBLIC_WORK_DAY_HOLIDAY,
    PUBLIC_WORK_DAY_WORKDAY,
} PublicWorkDay;

typedef enum {
    COMMUTE_PERIOD_GO_TO_WORK,
    COMMUTE_PERIOD_LEAVE_COMPANY,
    COMMUTE_PERIOD_ARRIVE_HOME,
    COMMUTE_PERIOD_LEAVE_HOME,
    COMMUTE_PERIOD_ARRIVE_COMPANY,
} CommutePeriod;

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
```

### 4.2 场景配置: SceneConfig

```c
typedef enum {
    BACKTRACK_STRATEGY_BY_APP_FIRST_ACTIVE,
    BACKTRACK_STRATEGY_BY_APP_LAST_ACTIVE,
    BACKTRACK_STRATEGY_BY_STATE_CHANGE,
    BACKTRACK_STRATEGY_SELF_AS_INTENT,
} BacktrackStrategy;

// 字段类型枚举（替代字符串数组，更节省内存）
typedef enum {
    FIELD_SNAPSHOT_TIME,
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

// 场景字段视图配置
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

// 单条场景规则
typedef struct {
    char *sceneName;           // "food_ordering" 等
    SceneFieldView fieldView;  // 字段视图配置
    BacktrackStrategy strategy; // 回溯策略
} SceneRule;

// APP→场景映射条目
typedef struct {
    char *appPackage;    // "com.sankuai.meituan"
    char *sceneName;     // "food_ordering"
} AppSceneMapEntry;

// 全局场景配置
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
```

### 4.3 输出: IntentCandidate & IntentResult

```c
typedef struct {
    Snapshot *intentSnapshot;     // 意图发生时刻快照
    Snapshot *evidenceList;       // 证据快照数组
    int evidenceCount;            // 证据数量（纯规则候选可为0）
    char *sceneName;              // 命中的场景名称
} IntentCandidate;

typedef struct {
    IntentCandidate *candidateList;
    int candidateCount;
} IntentResult;
```

## 5. 核心流程

### 5.1 主入口

```c
/**
 * 处理一个用户的快照批次，输出候选意图样本
 * @param SnapshotList    快照数组（已按时间升序排列）
 * @param SnapshotCount   快照数量
 * @param Result          输出结果（调用方预分配）
 * @return 0成功, 负数失败
 */
int IntentFilterProcess(Snapshot *SnapshotList, int SnapshotCount,
                        IntentResult *Result);
```

内部流程:
1. 验证输入参数
2. 遍历 SnapshotList 中每个快照
3. 调用 `MatchSceneRules()` 检查是否命中场景
4. 对每个命中的场景，调用 `BacktrackIntentSnapshot()` 查找意图快照
5. 对每个意图快照，调用 `CollectEvidence()` 收集证据
6. 调用 `FillCaption()` 填充caption
7. 将结果写入 Result

### 5.2 规则匹配

```c
/**
 * 检查快照是否命中任何场景规则，返回命中的规则列表
 * @param Snapshot        当前快照
 * @param Config          场景配置
 * @param MatchedRules    输出命中规则数组
 * @param MatchedCount    输出命中数量
 * @return 0成功, 负数失败
 */
int MatchSceneRules(Snapshot *Snapshot, SceneConfig *Config,
                    SceneRule **MatchedRules, int *MatchedCount);
```

匹配逻辑（命中任一即生成候选锚点）:
- **APP匹配**: 遍历 `appUsage30min`，查 `appSceneMap` 是否有映射
- **POI匹配**: 检查 `poiToday` 的 poiType 是否与场景相关
- **通勤匹配**: 检查 `commutePeriod` 是否与场景相关（如 check_in→ARRIVE_COMPANY）
- **通知匹配**: 检查 `notifications10min` 的 source 是否在 appSceneMap 中
- **时间段匹配**: 检查 `publicTimePeriod` / `publicWorkDay` 是否与场景相关

### 5.3 回溯查找意图快照

```c
/**
 * 在快照列表中回溯查找意图发生时刻快照
 * @param SnapshotList    快照数组
 * @param StartIdx        当前触发快照的索引
 * @param Rule            命中的场景规则
 * @param IntentIdx       输出意图快照索引
 * @return 0找到意图, 1未找到(丢弃该候选), 负数异常
 */
int BacktrackIntentSnapshot(Snapshot *SnapshotList, int StartIdx,
                            SceneRule *Rule, int *IntentIdx);
```

回溯策略:

| 策略 | 触发条件 | 回溯行为 |
|------|----------|----------|
| `BACKTRACK_STRATEGY_BY_APP_FIRST_ACTIVE` | APP使用命中 | 在 lookBackMs 窗口内找场景APP首次活跃快照 |
| `BACKTRACK_STRATEGY_BY_APP_LAST_ACTIVE` | APP使用命中 | 在 lookBackMs 窗口内找场景APP最近一次活跃快照 |
| `BACKTRACK_STRATEGY_BY_STATE_CHANGE` | POI/通勤状态命中 | 在 lookBackMs 窗口内找POI/通勤状态发生变化的快照 |
| `BACKTRACK_STRATEGY_SELF_AS_INTENT` | 快照同时满足触发与证据 | 当前快照作为意图快照 |

场景与回溯策略的映射（由匹配条件动态决定）:
- 如果快照命中了 APP匹配 → 使用 `BACKTRACK_STRATEGY_BY_APP_FIRST_ACTIVE`
- 如果快照命中了 POI/通勤/时间段匹配（无APP匹配） → 使用 `BACKTRACK_STRATEGY_BY_STATE_CHANGE`
- 如果快照同时命中了 APP匹配 + 其他匹配 → 使用 `BACKTRACK_STRATEGY_BY_APP_FIRST_ACTIVE`（APP优先）
- 如果回溯窗口内找不到符合策略的快照 → 丢弃该候选

回溯窗口按时间范围筛选: `[snapshotTime - lookBackMs, snapshotTime]`

### 5.4 证据收集

```c
/**
 * 收集与意图快照相关的证据快照列表
 * @param SnapshotList     快照数组
 * @param StartIdx         触发快照索引
 * @param IntentIdx        意图快照索引
 * @param Rule             场景规则
 * @param EvidenceList     输出证据快照数组
 * @param EvidenceCount    输出证据数量
 * @return 0成功, 负数失败
 */
int CollectEvidence(Snapshot *SnapshotList, int StartIdx, int IntentIdx,
                    SceneRule *Rule, Snapshot **EvidenceList,
                    int *EvidenceCount);
```

收集逻辑:
1. 确定 `[intentTime - lookBackMs, intentTime + lookForwardMs]` 时间窗口
2. 遍历窗口内所有快照
3. 对每个快照，检查其 evidenceFields 是否与当前场景相关（通知、APP、POI等）
4. 只保留包含与该场景相关信息的快照
5. 纯规则候选允许 evidenceCount = 0

### 5.5 Caption填充

```c
/**
 * 将记忆表中的caption填充回证据快照的对应字段
 * @param EvidenceList     证据快照数组
 * @param EvidenceCount    证据数量
 */
void FillCaption(Snapshot *EvidenceList, int EvidenceCount);
```

从 `latestCompanionMemoryInfo` 和 `latestScreenMemoryInfo` 的 `entityId` 字段检索记忆表，获取完整 caption 填充到对应证据快照。

## 6. 错误处理与内存管理

### 6.1 错误码

```c
#define INTENT_FILTER_OK           0
#define INTENT_FILTER_ERR_PARAM    -1
#define INTENT_FILTER_ERR_MEMORY   -2
#define INTENT_FILTER_ERR_PARSE    -3
#define INTENT_FILTER_ERR_NO_RULE  -4
```

### 6.2 日志宏

```c
#define LOG_ERROR(fmt, ...)   // 适配目标平台日志系统
#define LOG_WARN(fmt, ...)
#define LOG_INFO(fmt, ...)
```

### 6.3 内存管理原则

1. 调用方负责分配 `IntentResult` 缓冲区，服务方负责填充
2. 内部临时内存（matchedRules、evidenceList）用完即释
3. 异常分支使用 goto-free 模式释放已分配内存
4. JSON解析产生的字符串内存统一由 `SnapshotDestroy()` 函数释放

```c
/**
 * 释放Snapshot内部所有动态分配的内存
 * @param Snapshot  要释放的快照
 */
void SnapshotDestroy(Snapshot *Snapshot);
```

## 7. 文件结构

```
intent/
├── src/
│   ├── intent_filter.c      — 主入口与流程编排
│   ├── scene_rule.c         — 场景规则匹配
│   ├── backtrack.c          — 回溯查找
│   ├── evidence.c           — 证据收集
│   ├── snapshot_parser.c    — JSON→Snapshot解析
│   ├── scene_config.c       — 场景配置加载
│   ├── cJSON.c              — 第三方JSON库
├── include/
│   ├── intent_filter.h
│   ├── scene_rule.h
│   ├── backtrack.h
│   ├── evidence.h
│   ├── snapshot_parser.h
│   ├── scene_config.h
│   ├── cJSON.h
│   ├── intent_types.h       — 所有类型定义集中
├── test/
│   ├── test_intent_filter.c
│   ├── test_scene_rule.c
│   ├── test_backtrack.c
│   ├── test_evidence.c
│   ├── test_snapshot_parser.c
│   ├── test_scene_config.c
│   ├── test_data/           — 测试用JSON数据
├── docs/
│   └── superpowers/
│       └─ specs/
├── desin.md
└── CLAUDE.md
```

## 8. 测试策略

- 每个模块独立单元测试
- 测试数据: 构造覆盖各场景的JSON快照数据
- 关键测试点:
  - 规则匹配: 各字段（APP、POI、通勤、通知、时间段）的命中/未命中
  - 回溯: 各策略的正确回溯结果
  - 证据收集: 时间窗口范围、相关性筛选、空证据候选
  - Caption填充: entityId→caption的映射
  - 内存安全: 正常流程和异常流程下无内存泄漏
  - 边界: 空输入、单快照、回溯窗口为0、多个候选场景
