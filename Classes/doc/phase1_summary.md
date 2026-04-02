# Phase 1 完成总结 — 交接给 Phase 2

## 一、当前状态

**Phase 1 已完成，程序可编译运行，画面静态显示正常。**

可执行文件：`build/bin/CardGame/Debug/CardGame.exe`

---

## 二、已确认的设计决策（重要，不可改变）

### 坐标系
- cocos2d 原点在**屏幕左下角**，y 向上
- JSON 配置坐标 `Position` 是**牌的左上角**坐标，单位像素，y 向下（设计工具坐标系）
- `GameModelFromLevelGenerator` 中已做转换：`card.position = cfg.position + Vec2(kCardWidth/2, kCardHeight/2)`，存入 `CardModel.position` 的是**中心点坐标**，仍为 y-down
- `GameController::_populateViews` 直接将 `card.position` 传给 `PlayFieldView::addCardView`（即直接用于 cocos2d 定位），**不做 y-inversion**，因用户已在外部调整好了坐标系

### 卡牌尺寸（绝对像素，不缩放）
- `CardView::kCardWidth  = 182.0f`
- `CardView::kCardHeight = 282.0f`

### 布局
- 设计分辨率：1080 × 2080，窗口 scale = 0.5（实际窗口 540 × 1040）
- `PlayFieldView`：position = `(0, 580)`，尺寸 1080 × 1500
- `StackView`（备用牌堆）：position 已由用户微调，左下区域
- `TrayView`（手牌区）：position 已由用户微调，右下区域
- Undo 按钮：`Label`，position `(540, 80)`

### 遮挡关系规则
- **后放的牌（id 更大）覆盖先放的牌（id 更小）**
- 只要两张牌矩形相交，且 j.id > i.id，则 i 被 j 遮挡（`coveredByIds`）
- **z-order = card.id**（id 越大渲染越靠前）
- 遮挡计算在 `GameModelFromLevelGenerator::_computeCoverRelations` 完成

### 匹配规则（Phase 2 需实现）
- 点数差 1 即可匹配，无花色限制
- K(12) 与 A(0) 循环匹配（diff == 1 || diff == 12）

---

## 三、已创建的文件清单

```
Classes/
├── utils/
│   └── CardEnums.h                    # CardFaceType / CardSuitType 枚举
├── configs/
│   ├── models/
│   │   ├── LevelConfig.h              # 关卡静态配置结构（CardConfig, LevelConfig）
│   │   └── CardResConfig.h/.cpp       # 资源路径映射（静态方法）
│   └── loaders/
│       └── LevelConfigLoader.h/.cpp   # JSON → LevelConfig（rapidjson）
├── models/
│   ├── CardModel.h                    # 单牌数据（id, face, suit, position, faceUp, coveredByIds）
│   └── GameModel.h/.cpp               # 全局数据（playfieldCardIds, stackCardIds, trayCardIds, allCards）
├── services/
│   └── GameModelFromLevelGenerator.h/.cpp  # LevelConfig → GameModel，遮挡关系计算
├── views/
│   ├── CardView.h/.cpp                # 单牌视图（正/背面拼接，Phase 1 动画为空实现）
│   ├── PlayFieldView.h/.cpp           # 主牌区容器，管理 CardView，转发点击回调
│   ├── StackView.h/.cpp               # 备用牌堆（扇形展开，背面），点击回调
│   ├── TrayView.h/.cpp                # 手牌区（始终显示顶部一张），含 cardStack 历史栈
│   └── GameView.h/.cpp                # 顶层容器，布局 + Undo 按钮
├── controllers/
│   └── GameController.h/.cpp          # Phase 1 最小实现：_initModel + _initView + _populateViews
├── AppDelegate.h/.cpp
└── HelloWorldScene.h/.cpp
Resources/
└── levels/
    └── level1.json                    # 20张主牌 + 9张备用牌的完整关卡配置
```

---

## 四、关键数据流（已验证）

```
level1.json
  → LevelConfigLoader::loadFromFile()        → LevelConfig
  → GameModelFromLevelGenerator::generate()  → GameModel
      ├── 位置加半牌偏移（→中心坐标）
      ├── _computeCoverRelations()（j>i 后放覆盖先放）
      ├── faceUp = coveredByIds.empty()
      ├── Stack[0] → trayCardIds（初始底牌，正面）
      └── Stack[1..] → stackCardIds（备用牌堆，背面）
  → GameController::_populateViews()
      ├── 每张 playfieldCard → CardView，z=card.id，加入 PlayFieldView
      ├── stackCardIds.size() → StackView::initStack()
      └── trayCardIds[0] → CardView（showFront）→ TrayView::setTopCard()
```

---

## 五、Phase 1 遗留的空实现（Phase 2 必须补全）

| 位置 | 内容 |
|------|------|
| `CardView::playFlipToFrontAnimation` | 直接 showFront()，无动画 |
| `CardView::playMoveToAnimation` | 有 MoveTo 但无真实翻牌动画 |
| `GameController` | 无点击回调绑定，无子控制器，无 Undo |
| `GameView` | Undo 按钮存在但回调未绑定 |

---

## 六、Phase 2 开发目标

详细计划见 `Classes/doc/implementation_plan.md` Phase 2 节，核心任务：

1. **`services/MatchRuleService.h/.cpp`** — `canMatch(a, b)` 静态方法
2. **`controllers/PlayFieldController.h/.cpp`** — 点击匹配全流程
3. **`controllers/StackController.h/.cpp`** — 翻备用牌流程
4. **补全 `CardView` 动画** — `playFlipToFrontAnimation`（X轴缩放翻转）、`playMoveToAnimation` 已基本实现
5. **补全 `GameController`** — `_initControllers`, `_bindCallbacks`, `_isAnimating` 锁, `_checkGameState`
6. **`GameController::handleUndo`** 先留桩，Phase 3 实现

### Phase 2 核心难点提醒
- **动画锁** `_isAnimating`：飞行动画期间所有点击必须被屏蔽，在所有回调入口处检查
- **revealedCardIds 回填**：`_revealUncoveredCards` 在动画完成后执行，需回填到刚 push 的 UndoRecord（`undoStack.back().revealedCardIds`）
- **坐标转换**：CardView 从 PlayFieldView 飞向 TrayView 需跨父节点，使用 `convertToWorldSpace` / `convertToNodeSpace`

---

## 七、编译环境备注

- cocos2d-x 3.17，cmake 构建，MSVC
- 已在 CMakeLists.txt 添加 `/utf-8` 编译选项（中文注释需要）
- 所有图片资源在 `Resources/views/`，由 cmake 自动 copy 到 build 目录
