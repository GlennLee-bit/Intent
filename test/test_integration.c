/* test/test_integration.c */
#include "test_helpers.h"
#include "intent_filter.h"
#include "snapshot_parser.h"
#include "scene_config.h"
#include <stdio.h>

void TestEndToEndSingleScenario(void) {
    SceneConfig config;
    SceneConfigLoadFromFile("test/test_data/scene_config.json", &config);
    IntentFilterInit(&config);
    Snapshot snapshots[2];
    SnapshotParseFromFile("test/test_data/snapshot_food.json", &snapshots[0]);
    SnapshotParseFromFile("test/test_data/snapshot_food.json", &snapshots[1]);
    snapshots[1].snapshotTime = snapshots[0].snapshotTime + 300000;
    IntentResult result;
    IntentCandidate candidates[MAX_INTENT_CANDIDATES];
    result.candidateList = candidates;
    result.candidateCount = 0;
    int ret = IntentFilterProcess(snapshots, 2, &result);
    TEST_ASSERT(ret == INTENT_FILTER_OK);
    printf("Found %d candidates\n", result.candidateCount);
    IntentResultDestroy(&result);
    SnapshotDestroy(&snapshots[0]);
    SnapshotDestroy(&snapshots[1]);
    SceneConfigDestroy(&config);
}

int main(void) {
    TestEndToEndSingleScenario();
    TEST_SUMMARY();
    return 0;
}