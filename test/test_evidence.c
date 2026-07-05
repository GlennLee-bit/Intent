/* test/test_evidence.c */
#include "test_helpers.h"
#include "evidence.h"
#include "snapshot_parser.h"
#include "scene_config.h"
#include <string.h>

void TestCollectEvidence(void) {
    Snapshot snapList[2];
    SnapshotParseFromFile("test/test_data/snapshot_sequence.json", &snapList[0]);
    snapList[1] = snapList[0];
    snapList[1].snapshotTime = 1781661450000LL;
    SceneConfig config;
    SceneConfigLoadFromFile("test/test_data/scene_config.json", &config);
    SceneRule *rule = &config.sceneRules[0];
    Snapshot evidence[MAX_EVIDENCE_PER_INTENT];
    int evidenceCount = 0;
    int ret = CollectEvidence(snapList, 2, 1, 0, rule, evidence, &evidenceCount);
    TEST_ASSERT(ret == INTENT_FILTER_OK);
    TEST_ASSERT(evidenceCount >= 0);
    /* evidence 是快照指针数组，不需要 deep destroy */
    SnapshotDestroy(&snapList[0]);
    SceneConfigDestroy(&config);
}

int main(void) {
    TestCollectEvidence();
    TEST_SUMMARY();
    return 0;
}