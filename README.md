# 序列筛选与特征关联

嵌入式 C 语言实现的实时意图筛选服务。

## 构建

```bash
make all
```

## 运行测试

```bash
make test           # 单元测试
make integration    # 集成测试
make clean          # 清理构建产物
```

## 使用示例

```c
#include "intent_filter.h"
#include "scene_config.h"
#include "snapshot_parser.h"

SceneConfig config;
SceneConfigLoadFromFile("path/to/scene_config.json", &config);
IntentFilterInit(&config);

Snapshot snapshots[10];
/* ... 加载快照 ... */

IntentResult result;
IntentCandidate candidates[MAX_INTENT_CANDIDATES];
result.candidateList = candidates;
IntentFilterProcess(snapshots, 10, &result);

for (int i = 0; i < result.candidateCount; i++) {
    printf("Intent %d: scene=%s, evidence=%d\n",
           i, result.candidateList[i].sceneName,
           result.candidateList[i].evidenceCount);
}

IntentResultDestroy(&result);
```

## 项目结构

```
intent/
├── src/                业务源文件
├── include/            公共头文件
├── test/               测试代码与测试数据
├── third_party/cJSON/  cJSON 库
├── docs/superpowers/   设计与计划文档
└── Makefile            构建脚本
```

## 编码规范

- 函数: 大驼峰 (`IntentFilterProcess`)
- 变量: 首字母小写无下划线 (`snapshotCount`)
- 类型: 首字母大写无下划线 (`Snapshot`)
- 枚举值: 全大写带下划线 (`PUBLIC_TIME_PERIOD_NOON`)
- 函数 ≤ 50 行
- 异常分支必须释放内存并打印日志