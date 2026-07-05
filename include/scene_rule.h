#ifndef SCENE_RULE_H
#define SCENE_RULE_H

#include "intent_types.h"

#define MAX_MATCHED_RULES 16

/**
 * 检查快照是否命中任何场景规则
 * @param Snapshot       当前快照
 * @param Config         场景配置
 * @param MatchedRules   输出命中规则数组（容量需 >= MAX_MATCHED_RULES）
 * @param MatchedCount   输出命中数量
 * @return INTENT_FILTER_OK 成功
 */
int MatchSceneRules(Snapshot *Snapshot, SceneConfig *Config,
                    SceneRule *MatchedRules, int *MatchedCount);

/**
 * 根据规则和匹配条件动态决定回溯策略
 * @param Rule         命中的场景规则
 * @param HasAppMatch  是否命中了 APP 匹配
 * @return 回溯策略
 */
BacktrackStrategy DecideBacktrackStrategy(SceneRule *Rule, int HasAppMatch);

#endif /* SCENE_RULE_H */