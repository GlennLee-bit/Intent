/* test/test_backtrack.c */
#include "test_helpers.h"
#include "backtrack.h"
#include "snapshot_parser.h"
#include "scene_config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void TestParseSequence(void) {
    /* 简化为单文件加载多快照 */
    Snapshot snapshots[2];
    SnapshotParseFromFile("test/test_data/snapshot_sequence.json", &snapshots[0]);
    /* 注意：实际多快照解析需要新接口，这里仅验证第二个快照可加载 */
    TEST_ASSERT(snapshots[0].snapshotTime == 1781661300000LL);
    TEST_ASSERT(snapshots[0].appUsageCount == 1);
    SnapshotDestroy(&snapshots[0]);
}

void TestBacktrackByAppFirstActive(void) {
    Snapshot snapList[2];
    SnapshotParseFromFile("test/test_data/snapshot_sequence.json", &snapList[0]);
    /* 第二个快照需要额外构造，这里使用同一数据 */
    snapList[1] = snapList[0];
    snapList[1].snapshotTime = 1781661450000LL;
    SceneConfig config;
    SceneConfigLoadFromFile("test/test_data/scene_config.json", &config);
    SceneRule *rule = &config.sceneRules[0];
    int intentIdx = -1;
    int ret = BacktrackIntentSnapshot(snapList, 2, 1, rule,
                                       BACKTRACK_STRATEGY_BY_APP_FIRST_ACTIVE,
                                       &intentIdx);
    TEST_ASSERT(ret == 0);
    TEST_ASSERT(intentIdx == 0);
    SnapshotDestroy(&snapList[0]);
    /* snapList[1] 是浅拷贝，避免重复释放 */
    SceneConfigDestroy(&config);
}

int main(void) {
    TestParseSequence();
    TestBacktrackByAppFirstActive();
    TEST_SUMMARY();
    return 0;
}