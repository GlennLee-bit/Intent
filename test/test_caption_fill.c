/* test/test_caption_fill.c */
#include "test_helpers.h"
#include "caption_fill.h"
#include "snapshot_parser.h"
#include <string.h>

void TestFillCaptionNoop(void) {
    Snapshot snap;
    SnapshotParseFromFile("test/test_data/snapshot_food.json", &snap);
    /* 暂未构造 companionMemory，验证函数不会崩溃 */
    FillCaption(&snap, 1);
    TEST_ASSERT(1 == 1);
    SnapshotDestroy(&snap);
}

int main(void) {
    TestFillCaptionNoop();
    TEST_SUMMARY();
    return 0;
}