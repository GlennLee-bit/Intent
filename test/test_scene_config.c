/* test/test_scene_config.c */
#include "test_helpers.h"
#include "scene_config.h"
#include <string.h>

void TestLoadSceneConfig(void) {
    SceneConfig config;
    int ret = SceneConfigLoadFromFile("test/test_data/scene_config.json", &config);
    TEST_ASSERT(ret == INTENT_FILTER_OK);
    TEST_ASSERT(config.businessSceneCount == 3);
    TEST_ASSERT(strcmp(config.businessScenes[0], "food_ordering") == 0);
    TEST_ASSERT(config.appSceneMapCount == 3);
    TEST_ASSERT(strcmp(config.appSceneMap[0].appPackage, "com.sankuai.meituan") == 0);
    TEST_ASSERT(strcmp(config.appSceneMap[0].sceneName, "food_ordering") == 0);
    TEST_ASSERT(config.paymentAppCount == 2);
    TEST_ASSERT(config.sceneRuleCount >= 1);
    SceneConfigDestroy(&config);
}

int main(void) {
    TestLoadSceneConfig();
    TEST_SUMMARY();
    return 0;
}