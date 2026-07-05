/* test/test_snapshot_parser.c */
#include "test_helpers.h"
#include "snapshot_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void TestParseFoodSnapshot(void) {
    const char *filePath = "test/test_data/snapshot_food.json";
    Snapshot snapshot;
    int ret = SnapshotParseFromFile(filePath, &snapshot);
    TEST_ASSERT(ret == INTENT_FILTER_OK);
    TEST_ASSERT(snapshot.snapshotTime == 1781661600000LL);
    TEST_ASSERT(snapshot.publicTimePeriod == PUBLIC_TIME_PERIOD_NOON);
    TEST_ASSERT(snapshot.publicWorkDay == PUBLIC_WORK_DAY_WORKDAY);
    TEST_ASSERT(snapshot.commutePeriod == COMMUTE_PERIOD_GO_TO_WORK);
    TEST_ASSERT(snapshot.poiTodayCount == 1);
    TEST_ASSERT(strcmp(snapshot.poiTodayList[0].poiType, "WORKPLACE") == 0);
    TEST_ASSERT(snapshot.appUsageCount == 1);
    TEST_ASSERT(strcmp(snapshot.appUsageList[0].appName, "肯德基") == 0);
    TEST_ASSERT(snapshot.notificationCount == 1);
    TEST_ASSERT(strcmp(snapshot.notificationList[0].content,
                       "肯德基优惠券即将过期") == 0);
    TEST_ASSERT(strcmp(snapshot.userProfile.character.name, "TestUser") == 0);
    SnapshotDestroy(&snapshot);
}

int main(void) {
    TestParseFoodSnapshot();
    TEST_SUMMARY();
    return 0;
}