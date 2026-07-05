当前需求名称为**序列筛选与特征关联**

编码规范：

1.符合C语言编码规范，函数使用大驼峰格式，变量名称使用首字母小写，不要带_，参数类型的定义使用首字母大写，不要带_

2.每个函数不超过50行

3.申请释放内存保证规范，特别是异常分支处理

4.异常分支需要有日志打印

5.尽可能有详细的功能注释

6.对于JSON数据的解析使用标准的函数



1.整体的需求流程：

输入快照批次snapshot

  |——按用户与snapshotTime升序排序

  |——遍历每一个snapshot

​           |——结构化字段匹配与文本证据匹配

​                    |——APP使用、POI、通勤状态、时间段、通知、caption等字段命中任一场景规则

​                             ->生成候选evidence锚点
​                    |——未命中任何场景规则
​                             ->跳过当前快照

​            |——根据命中的规则集合确定候选场景

​                     |——场景命中

​                             ->进入该场景的回溯窗口

​              |——针对每个候选场景，回溯查找意图发生时刻快照

​                      |——若存在明确APP启动或活跃行为

​                             ->选择场景APP首次或最近一次有效活跃快照

​                      |——若场景由位置或时空状态触发

​                             ->选择POI、围栏、通勤状态发生变化的快照

​                      |——若当前快照本身同时满足触发条件与证据条件

​                             ->当前快照作为意图发生快照

​                      |——无法找到可解释的意图发生快照

​                             ->丢弃该候选场景

​               |——收集与该意图快照相关的证据快照列表

​                      |——收集意图快照前后窗口内命中同一场景的通知、caption、App、POI、通勤状态快照

​                      |——证据列表只保留与候选场景相关的快照

​                      |——纯规则候选允许evidence为空数组

​               |——输出内部候选样本

​                      |——将从记忆表中检索到的caption填充回对应证据快照

​                      |——{"snapshot":意图发生时刻快照, "evidence":[相关证据快照1，相关证据快照2，。。。]}





2.数据格式

snapshot格式

上游以用户和时间范围为单位提供，每个快照包含snapshotTime和features字段，字段名遵循流式输出。

```json
{
  "snapshotTime": 1781661600000,
  "features": {
    "publicTimePeriod": "NOON",	// "MIDNIGHT", "MORNING", "FORENOON", "NOON", "AFTERNOON", "NIGHT"
    "publicWorkDay": "PUBLIC_WORKDAY",	// "PUBLIC_HOLIDAY", "PUBLIC_WORKDAY"
    "commutePeriod": "GO_TO_WORK",	// "LEAVE_COMPANY", "ARRIVE_HOME", "LEAVE_HOME", "ARRIVE_COMPANY"
    "poiToday": [
      {
        "poiType": "WORKPLACE",	// string
        "startTime": 1781661300000,
        "endTime": 1781661360000
      },
      {
        "poiType": "HOME",
        "startTime": 1781661420000,
        "endTime": 1781661460000
      }
    ],
    "appUsage30min": [
      {
        "appName": "肯德基",	// string
        "startTime": 1781661480000,
        "endTime": 1781661600000
      }
    ],
    // 10分钟内push通知，最多5条
    "notifications10min": [
      {
        "timestamp": 1781661500000,
        "content": "肯德基优惠券即将过期",	// string 
        "source": "com.kfc.app"
      }
    ],
    // 5分钟内剪贴板，最多5条
    "pasteBoard5min": [
      {
        "timestamp": 1781661520000,
        "content": "KFC 优惠码",	// string
        "source": "com.chat.app"
      }
    ],
    "companionSummary": {
        "content": "String"
    },
    // 最近的15条伴随记忆
	"latestCompanionMemoryInfo": [{
		"entityId": "String",	// 主键，用于从伴随记忆表中检索
        "caption" : "String",
        "entity" : "String",
		"timestamp": "Number (Integer, Timestamp)",	// 屏幕截图时间，recall表中lastInteractionTimestamp
	},
    {
        ...
    }
    ],
    // 最近的15条帮记
	"latestScreenMemoryInfo": [{
		"entityId": "String",	// 主键，用于从帮记表中检索
        "caption" : "String",
        "entity" : "String",
		"timestamp": "Number (Integer, Timestamp)",	// 屏幕截图时间，recall表中lastInteractionTimestamp
	},
    {
        ...
    }
    ],
    "userProfile": {
        // UG character表
        "character": {
            "age": "...",
            "name": "..."
        },
        // L3记忆
        "user.md": "String"
    }
  }
}
```

场景配制

```json
{
  "businessScenes": ["food_ordering", "coffee_ordering", "navigation", "check_in", "grocery_purchase"],
  "appSceneMap": {		
    // 场景App白名单示例
    "com.sankuai.meituan": "food_ordering",
    "com.yum.kfc": "food_ordering",
    "com.luckin.client": "coffee_ordering",
    "com.starbucks.cn": "coffee_ordering",
    "com.autonavi.minimap": "navigation",
    "com.alibaba.android.rimet": "check_in",
    "com.ss.android.lark": "check_in",
    "com.yaya.zone": "grocery_purchase",
    "com.wudaokou.hippo": "grocery_purchase"
  },
  // 支付App白名单示例
  "paymentApps": ["com.tencent.mm", "com.eg.android.AlipayGphone"],
  "sceneFieldViews": {
    "food_ordering": {
      "evidenceFields": ["notifications10min"],
      "stateFields": ["appUsage30min"],
      "contextFields": ["snapshotTime", "publicTimePeriod", "poiToday", "latestCompanionMemoryInfo", "latestScreenMemoryInfo"],
      "lookBackMs": 600000,
      "lookForwardMs": 120000
    },
    "coffee_ordering": {
      "evidenceFields": ["notifications10min"],
      "stateFields": ["appUsage30min"],
      "contextFields": ["snapshotTime", "publicTimePeriod", "poiToday", "latestCompanionMemoryInfo", "latestScreenMemoryInfo"],
      "lookBackMs": 600000,
      "lookForwardMs": 120000
    },
    "navigation": {
      "evidenceFields": ["notifications10min"],
      "stateFields": ["appUsage30min"],
      "contextFields": ["snapshotTime", "publicWorkDay", "commutePeriod", "poiToday", "latestCompanionMemoryInfo", "latestScreenMemoryInfo"],
      "lookBackMs": 300000,
      "lookForwardMs": 300000
    },
    "check_in": {
      "evidenceFields": ["notifications10min"],
      "stateFields": ["appUsage30min"],
      "contextFields": ["snapshotTime", "publicWorkDay", "poiToday", "latestCompanionMemoryInfo", "latestScreenMemoryInfo"],
      "lookBackMs": 300000,
      "lookForwardMs": 300000
    },
    "grocery_purchase": {
      "evidenceFields": ["notifications10min"],
      "stateFields": ["appUsage30min"],
      "contextFields": ["snapshotTime", "poiToday", "latestCompanionMemoryInfo", "latestScreenMemoryInfo"],
      "lookBackMs": 600000,
      "lookForwardMs": 120000
    }
  }
}
```







