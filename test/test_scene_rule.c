/* test/test_scene_rule.c */
#include "test_helpers.h"
#include "scene_rule.h"
#include "scene_config.h"
#include "snapshot_parser.h"
#include <string.h>

static SceneConfig gConfig;

void SetupConfig(void) {
    int ret = SceneConfigLoadFromFile("test/test_data/scene_config.json", &gConfig);
    TEST_ASSERT(ret == INTENT_FILTER_OK);
}

void TeardownConfig(void) {
    SceneConfigDestroy(&gConfig);
}

void TestMatchByApp(void) {
    Snapshot snap;
    SnapshotParseFromFile("test/test_data/snapshot_food.json", &snap);
    SceneRule matched[MAX_MATCHED_RULES];
    int matchedCount = 0;
    int ret = MatchSceneRules(&snap, &gConfig, matched, &matchedCount);
    TEST_ASSERT(ret == INTENT_FILTER_OK);
    TEST_ASSERT(matchedCount >= 1);
    SnapshotDestroy(&snap);
}

void TestDecideStrategy(void) {
    SceneRule *rule = &gConfig.sceneRules[0];
    BacktrackStrategy s1 = DecideBacktrackStrategy(rule, 1);
    TEST_ASSERT(s1 == BACKTRACK_STRATEGY_BY_APP_FIRST_ACTIVE);
    BacktrackStrategy s2 = DecideBacktrackStrategy(rule, 0);
    TEST_ASSERT(s2 == BACKTRACK_STRATEGY_BY_STATE_CHANGE);
}

int main(void) {
    SetupConfig();
    TestMatchByApp();
    TestDecideStrategy();
    TeardownConfig();
    TEST_SUMMARY();
    return 0;
}