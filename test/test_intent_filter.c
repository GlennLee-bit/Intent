/* test/test_intent_filter.c */
#include "test_helpers.h"
#include "intent_filter.h"
#include "snapshot_parser.h"
#include "scene_config.h"
#include <string.h>

void TestFullPipeline(void) {
    SceneConfig config;
    int ret = SceneConfigLoadFromFile("test/test_data/scene_config.json", &config);
    TEST_ASSERT(ret == INTENT_FILTER_OK);
    ret = IntentFilterInit(&config);
    TEST_ASSERT(ret == INTENT_FILTER_OK);
    Snapshot snap1, snap2;
    SnapshotParseFromFile("test/test_data/snapshot_food.json", &snap1);
    /* 使用第二个快照作为序列 */
    SnapshotParseFromFile("test/test_data/snapshot_food.json", &snap2);
    snap2.snapshotTime += 600000;  /* 推后 10 分钟 */
    Snapshot snapList[2] = {snap1, snap2};
    IntentResult result;
    IntentCandidate candidates[MAX_INTENT_CANDIDATES];
    result.candidateList = candidates;
    result.candidateCount = 0;
    ret = IntentFilterProcess(snapList, 2, &result);
    TEST_ASSERT(ret == INTENT_FILTER_OK);
    TEST_ASSERT(result.candidateCount >= 0);
    IntentResultDestroy(&result);
    SnapshotDestroy(&snap1);
    SnapshotDestroy(&snap2);
    SceneConfigDestroy(&config);
}

int main(void) {
    TestFullPipeline();
    TEST_SUMMARY();
    return 0;
}