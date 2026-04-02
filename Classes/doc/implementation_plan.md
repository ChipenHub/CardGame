# 纸牌游戏 - 实现计划

> 本文档为 Sonnet 工程模型的可执行开发计划，按 Phase 顺序编码，每个 Phase 完成后编译运行验收再进入下一阶段。

---

## 一、全局约定

### 1.1 设计分辨率与布局

- 设计分辨率：1080 × 2080
- 主牌区：1080 × 1500（屏幕上半部分）
- 底部区域：1080 × 580（左侧备用牌堆 + 右侧手牌/底牌区）

### 1.2 卡牌素材（绝对像素，不缩放）

- `views/card_bg.png` — 白色底牌（即卡牌正面底色）
- `views/card_covered.png` — 牌背（蓝色）
- `views/number/big_{red|black}_{A|2-10|J|Q|K}.png` — 大数字，放牌面中心偏下
- `views/number/small_{red|black}_{A|2-10|J|Q|K}.png` — 小数字，放牌面左上角
- `views/suits/{club|diamond|heart|spade}.png` — 花色图标，放牌面右上角

颜色规则：梅花(clubs)/黑桃(spades) = black，方块(diamonds)/红桃(hearts) = red

### 1.3 匹配规则

- 点数差值为 1 即可匹配，无花色限制
- K 与 A 可循环匹配（diff == 1 || diff == 12）

### 1.4 遮挡关系

- 无显式配置，由卡牌矩形位置重叠自动计算
- 规则：若牌 A 的矩形与牌 B 的矩形有交集，且 A.y > B.y（cocos2d 中 y 越大越靠上），则 B 被 A 遮挡
- 被遮挡的牌显示为 card_covered，当所有遮挡它的牌被消除后自动翻开

---

## 二、文件清单与数据结构

### 2.1 utils/CardEnums.h

全局共享的枚举定义，无 .cpp 文件。

```cpp
#ifndef __CARD_ENUMS_H__
#define __CARD_ENUMS_H__

// 花色类型
enum CardSuitType {
    CST_NONE = -1,
    CST_CLUBS,      // 梅花 → black
    CST_DIAMONDS,   // 方块 → red
    CST_HEARTS,     // 红桃 → red
    CST_SPADES,     // 黑桃 → black
    CST_NUM_CARD_SUIT_TYPES
};

// 点数类型
enum CardFaceType {
    CFT_NONE = -1,
    CFT_ACE,    // 0
    CFT_TWO,    // 1
    CFT_THREE,  // 2
    CFT_FOUR,   // 3
    CFT_FIVE,   // 4
    CFT_SIX,    // 5
    CFT_SEVEN,  // 6
    CFT_EIGHT,  // 7
    CFT_NINE,   // 8
    CFT_TEN,    // 9
    CFT_JACK,   // 10
    CFT_QUEEN,  // 11
    CFT_KING,   // 12
    CFT_NUM_CARD_FACE_TYPES
};

#endif
```

### 2.2 configs/models/LevelConfig.h

关卡静态配置，纯数据结构，无 .cpp。

```cpp
/**
 * 关卡配置数据结构
 * 由 LevelConfigLoader 从 JSON 解析生成，只读
 */
struct CardConfig {
    CardFaceType face;
    CardSuitType suit;
    cocos2d::Vec2 position; // Playfield 牌的绝对坐标；Stack 牌 position 无意义
};

struct LevelConfig {
    std::vector<CardConfig> playfieldCards; // 对应 JSON "Playfield"
    std::vector<CardConfig> stackCards;     // 对应 JSON "Stack"，[0]为初始底牌，[1..]为备用牌
};
```

### 2.3 configs/models/CardResConfig.h + .cpp

卡牌资源路径映射，全部为静态方法。

```cpp
/**
 * 根据花色和点数返回对应的资源文件路径
 * 纯静态工具类，无状态
 */
class CardResConfig {
public:
    // 返回 "red" 或 "black"
    static std::string getColorStr(CardSuitType suit);

    // 返回 "A", "2"..."10", "J", "Q", "K"
    static std::string getFaceStr(CardFaceType face);

    // 返回如 "views/number/big_red_A.png"
    static std::string getBigNumberPath(CardFaceType face, CardSuitType suit);

    // 返回如 "views/number/small_black_10.png"
    static std::string getSmallNumberPath(CardFaceType face, CardSuitType suit);

    // 返回如 "views/suits/heart.png"
    static std::string getSuitIconPath(CardSuitType suit);
};
```

实现要点：
- `getColorStr`: clubs/spades → "black"，diamonds/hearts → "red"
- `getFaceStr`: CFT_ACE→"A", CFT_TWO→"2", ..., CFT_TEN→"10", CFT_JACK→"J", CFT_QUEEN→"Q", CFT_KING→"K"
- 路径格式固定：`"views/number/big_" + color + "_" + face + ".png"`

### 2.4 configs/loaders/LevelConfigLoader.h + .cpp

```cpp
/**
 * 关卡配置加载器
 * 从 JSON 文件解析出 LevelConfig
 */
class LevelConfigLoader {
public:
    // 传入 JSON 文件路径，返回解析后的 LevelConfig
    // 使用 cocos2d 的 rapidjson 解析
    static LevelConfig loadFromFile(const std::string& filePath);
};
```

实现要点：
- 使用 `cocos2d::FileUtils::getInstance()->getStringFromFile(filePath)` 读取
- 使用 rapidjson 解析
- Playfield 数组中每个对象：`CardFace` → CardFaceType, `CardSuit` → CardSuitType, `Position.x/y` → Vec2
- Stack 数组同理

### 2.5 models/CardModel.h

```cpp
/**
 * 单张卡牌的运行时数据模型
 */
struct CardModel {
    int id;                          // 唯一标识（初始化时按顺序分配）
    CardFaceType face;
    CardSuitType suit;
    cocos2d::Vec2 position;          // 在主牌区的坐标（Stack牌无意义）
    bool faceUp;                     // 是否正面朝上
    std::vector<int> coveredByIds;   // 遮挡此牌的其他牌ID列表（仅 playfield 牌使用）
};
```

### 2.6 models/GameModel.h

```cpp
/**
 * 游戏全局运行时数据
 * 包含三个区域的牌数据
 */
struct GameModel {
    std::vector<CardModel> playfieldCards;  // 主牌区所有牌
    std::vector<int> stackCardIds;          // 备用牌堆（从底到顶的ID顺序）
    std::vector<int> trayCardIds;           // 手牌区/底牌区（栈，最后一个为顶部牌）

    // 所有卡牌的中心索引（用 id 快速查找 CardModel）
    std::unordered_map<int, CardModel> allCards;

    // 获取手牌区顶部牌ID，无牌返回 -1
    int getTrayTopCardId() const;

    // 根据ID获取CardModel引用
    CardModel& getCard(int id);
    const CardModel& getCard(int id) const;
};
```

### 2.7 models/UndoModel.h

```cpp
/**
 * 撤销记录
 */
struct UndoRecord {
    enum Type {
        MATCH,  // 主牌区匹配消除
        DRAW    // 从备用牌堆翻牌
    };

    Type type;
    int cardId;                        // 操作的牌ID
    cocos2d::Vec2 fromPosition;        // 牌的原始位置（MATCH时有意义）
    std::vector<int> revealedCardIds;  // 本次操作导致翻开的牌ID列表（MATCH时有意义）
    int previousTrayTopId;             // 操作前的手牌区顶部牌ID
};

/**
 * 撤销记录栈
 */
struct UndoModel {
    std::vector<UndoRecord> undoStack;

    bool canUndo() const { return !undoStack.empty(); }
    void pushRecord(const UndoRecord& record) { undoStack.push_back(record); }
    UndoRecord popRecord() {
        auto record = undoStack.back();
        undoStack.pop_back();
        return record;
    }
    void clear() { undoStack.clear(); }
};
```

### 2.8 views/CardView.h + .cpp

继承 `cocos2d::Node`。

```cpp
/**
 * 单张卡牌的视图
 * 由 card_bg + 大数字 + 小数字 + 花色图标 组合而成
 * 可切换正面/背面显示
 */
class CardView : public cocos2d::Node {
public:
    static CardView* create(int cardId, CardFaceType face, CardSuitType suit);
    bool init(int cardId, CardFaceType face, CardSuitType suit);

    int getCardId() const;

    // --- 显示状态 ---
    void showFront();   // 显示正面（card_bg + 数字 + 花色）
    void showBack();    // 显示背面（card_covered）

    // --- 动画 ---
    // 翻牌动画：背面 → 正面（带3D翻转效果，先X缩放到0再展开正面）
    void playFlipToFrontAnimation(float duration = 0.3f, std::function<void()> onComplete = nullptr);

    // 移动到目标位置动画
    void playMoveToAnimation(const cocos2d::Vec2& target, float duration = 0.3f, std::function<void()> onComplete = nullptr);

    // --- 点击 ---
    void setClickCallback(std::function<void(int cardId)> callback);
    void setClickEnabled(bool enabled);

private:
    int _cardId;
    CardFaceType _face;
    CardSuitType _suit;

    cocos2d::Sprite* _cardBg;        // 白色底牌
    cocos2d::Sprite* _coveredSprite;  // 牌背
    cocos2d::Sprite* _bigNumber;      // 大数字（中心偏下）
    cocos2d::Sprite* _smallNumber;    // 小数字（左上角）
    cocos2d::Sprite* _suitIcon;       // 花色（右上角）

    bool _isFront;
    bool _clickEnabled;
    std::function<void(int)> _clickCallback;

    void _setupFrontFace();  // 创建正面各精灵并定位
    void _setupBackFace();   // 创建背面精灵
    void _setupTouchListener();
};
```

**布局定位要点**（CardView 的锚点设为 (0.5, 0.5)）：
- `_cardBg`：position = (0, 0)，锚点(0.5, 0.5)，即铺满整个CardView
- `_bigNumber`：position = (0, -cardHeight * 0.1)，锚点(0.5, 0.5)，中心偏下
- `_smallNumber`：position = (-cardWidth/2 + 边距, cardHeight/2 - 边距)，锚点(0, 1)，左上角
- `_suitIcon`：position = (cardWidth/2 - 边距, cardHeight/2 - 边距)，锚点(1, 1)，右上角
- `_coveredSprite`：position = (0, 0)，覆盖整张牌

其中 cardWidth / cardHeight 由 card_bg.png 的实际像素尺寸决定（不缩放）。边距取 8~12px。

### 2.9 views/PlayFieldView.h + .cpp

```cpp
/**
 * 主牌区视图
 * 管理所有在主牌区的 CardView
 */
class PlayFieldView : public cocos2d::Node {
public:
    static PlayFieldView* create();

    // 添加卡牌到主牌区（初始化时调用）
    void addCardView(CardView* cardView, const cocos2d::Vec2& position);

    // 移除卡牌（匹配消除时，带飞行动画由 Controller 处理）
    void removeCardView(int cardId);

    // 获取指定卡牌视图
    CardView* getCardView(int cardId);

    // 设置卡牌点击回调（统一由此View转发给Controller）
    void setOnCardClickCallback(std::function<void(int cardId)> callback);

private:
    std::unordered_map<int, CardView*> _cardViews;
    std::function<void(int)> _onCardClickCallback;
};
```

### 2.10 views/StackView.h + .cpp

```cpp
/**
 * 备用牌堆视图
 * 扇形展开布局，全部背面朝上，每张牌水平偏移 kStackCardOffsetX
 */
class StackView : public cocos2d::Node {
public:
    static StackView* create();

    // 初始化备用牌堆显示（传入牌的数量，全部为背面）
    void initStack(int cardCount);

    // 移除顶部牌并返回其位置（用于飞行动画起点）
    // Controller 调用此方法获取起始位置后，创建动画让牌飞向 TrayView
    cocos2d::Vec2 popTopCard();

    // 获取剩余牌数
    int getRemainingCount() const;

    // 设置点击回调
    void setOnClickCallback(std::function<void()> callback);

private:
    std::vector<cocos2d::Sprite*> _stackCards;  // 背面牌精灵列表
    std::function<void()> _onClickCallback;

    static constexpr float kStackCardOffsetX = 18.0f;  // 每张牌的水平偏移

    void _setupTouchListener();
};
```

### 2.11 views/TrayView.h + .cpp

```cpp
/**
 * 手牌区/底牌区视图
 * 始终显示顶部一张正面牌
 */
class TrayView : public cocos2d::Node {
public:
    static TrayView* create();

    // 设置当前顶部牌（无动画，初始化用）
    void setTopCard(CardView* cardView);

    // 获取当前顶部牌视图
    CardView* getTopCardView() const;

    // 获取顶部牌的世界坐标（用于飞行动画终点计算）
    cocos2d::Vec2 getTopCardWorldPosition() const;

    // 播放新牌飞入动画（牌从外部飞入后成为新的顶部牌）
    void playCardFlyInAnimation(CardView* cardView, const cocos2d::Vec2& fromWorldPos,
                                 float duration = 0.3f, std::function<void()> onComplete = nullptr);

    // Undo 时：移除顶部牌，恢复为上一张
    void restorePreviousTopCard(CardView* cardView);

private:
    CardView* _topCardView;
    std::vector<CardView*> _cardStack;  // 所有手牌区的牌（用于Undo恢复）
};
```

### 2.12 views/GameView.h + .cpp

```cpp
/**
 * 游戏主视图
 * 顶层容器，管理布局：背景 + 主牌区 + 备用牌堆 + 手牌区 + 回退按钮
 */
class GameView : public cocos2d::Node {
public:
    static GameView* create();

    PlayFieldView* getPlayFieldView() const;
    StackView* getStackView() const;
    TrayView* getTrayView() const;

    // 设置回退按钮点击回调
    void setOnUndoClickCallback(std::function<void()> callback);

    // 设置回退按钮状态
    void setUndoButtonEnabled(bool enabled);

private:
    cocos2d::Sprite* _bgSprite;       // game_bg.png 背景
    PlayFieldView* _playFieldView;
    StackView* _stackView;
    TrayView* _trayView;
    cocos2d::ui::Button* _undoButton; // 回退按钮（可用 Label 模拟）

    void _initLayout();
};
```

**布局方案**：
- `_bgSprite`：全屏 1080×2080，position = (540, 1040)
- `_playFieldView`：position = (0, 580)，尺寸 1080×1500，主牌区卡牌的坐标相对于此 Node
- 底部区域 1080×580：
  - `_stackView`：position = (左半区中心x, 290)，备用牌堆
  - `_trayView`：position = (右半区中心x, 290)，手牌区
  - `_undoButton`：position = (底部区域某合适位置)

### 2.13 services/MatchRuleService.h + .cpp

```cpp
/**
 * 匹配规则判断
 * 纯静态无状态服务
 */
class MatchRuleService {
public:
    // 判断两张牌是否可匹配（点数差 1，K-A 可循环）
    static bool canMatch(CardFaceType a, CardFaceType b);
};
```

实现：
```cpp
bool MatchRuleService::canMatch(CardFaceType a, CardFaceType b) {
    if (a == CFT_NONE || b == CFT_NONE) return false;
    int diff = std::abs(static_cast<int>(a) - static_cast<int>(b));
    return diff == 1 || diff == (CFT_NUM_CARD_FACE_TYPES - 1); // 1 or 12
}
```

### 2.14 services/GameModelFromLevelGenerator.h + .cpp

```cpp
/**
 * 将静态 LevelConfig 转换为运行时 GameModel
 * 处理：ID分配、遮挡关系计算、初始 faceUp 设置
 */
class GameModelFromLevelGenerator {
public:
    static GameModel generate(const LevelConfig& config);

private:
    // 根据卡牌位置和尺寸计算遮挡关系
    // 规则：若 A.rect ∩ B.rect 非空 且 A.position.y > B.position.y，则 B.coveredByIds 包含 A.id
    static void _computeCoverRelations(std::vector<CardModel>& cards, float cardWidth, float cardHeight);
};
```

实现要点：
1. 依次为 Playfield 牌分配 id = 0, 1, 2, ...
2. Stack 牌继续分配 id
3. `_computeCoverRelations`：双重循环每对 Playfield 牌，计算矩形交集
   - 矩形 = (pos.x - w/2, pos.y - h/2, w, h)
   - 若有交集且 A.y > B.y → B.coveredByIds.push_back(A.id)
4. faceUp 判定：若 coveredByIds 为空 → faceUp = true，否则 false
5. Stack[0] 设为初始底牌放入 trayCardIds，其余放入 stackCardIds
6. 构建 allCards map

### 2.15 managers/UndoManager.h + .cpp

```cpp
/**
 * 撤销管理器
 * 持有 UndoModel，提供 push/pop 接口
 * 作为 GameController 的成员变量，禁止单例
 */
class UndoManager {
public:
    UndoManager();

    // 记录一次匹配操作
    void recordMatch(int cardId, const cocos2d::Vec2& fromPosition,
                     const std::vector<int>& revealedCardIds, int previousTrayTopId);

    // 记录一次翻牌操作
    void recordDraw(int cardId, int previousTrayTopId);

    // 执行撤销，返回撤销记录；若无记录返回 nullopt
    // Controller 根据返回的记录执行反向动画和数据恢复
    std::optional<UndoRecord> undo();

    bool canUndo() const;
    void clear();

private:
    UndoModel _undoModel;
};
```

### 2.16 controllers/PlayFieldController.h + .cpp

```cpp
/**
 * 主牌区控制器
 * 处理主牌区卡牌点击 → 匹配判断 → 数据更新 → 动画触发
 */
class PlayFieldController {
public:
    /**
     * 初始化
     * @param gameModel 游戏数据模型指针（由 GameController 持有）
     * @param undoManager 撤销管理器指针
     */
    void init(GameModel* gameModel, UndoManager* undoManager);

    // 初始化视图：创建 CardView 并添加到 PlayFieldView
    void initView(PlayFieldView* playFieldView, TrayView* trayView);

    /**
     * 处理卡牌点击
     * 1. 检查牌是否 faceUp
     * 2. 调用 MatchRuleService 判断是否可匹配
     * 3. 记录 Undo
     * 4. 更新 GameModel
     * 5. 触发动画（卡牌飞向 TrayView）
     * 6. 检查遮挡关系，翻开新暴露的牌
     * @return true 如果匹配成功
     */
    bool handleCardClick(int cardId);

    // 回退匹配操作（由 GameController 在 Undo 时调用）
    void undoMatch(const UndoRecord& record);

    // 设置动画锁回调（通知 GameController 动画开始/结束）
    void setAnimationLockCallback(std::function<void(bool locked)> callback);

    // 检查主牌区是否已清空
    bool isPlayFieldCleared() const;

    // 检查是否有可匹配的牌
    bool hasMatchableCard() const;

private:
    GameModel* _gameModel;
    UndoManager* _undoManager;
    PlayFieldView* _playFieldView;
    TrayView* _trayView;
    std::function<void(bool)> _animationLockCallback;

    // 消除卡牌后，检查并翻开不再被遮挡的牌，返回被翻开的牌ID列表
    std::vector<int> _revealUncoveredCards(int removedCardId);
};
```

### 2.17 controllers/StackController.h + .cpp

```cpp
/**
 * 备用牌堆控制器
 * 处理备用牌堆点击 → 翻牌 → 更新手牌区
 */
class StackController {
public:
    void init(GameModel* gameModel, UndoManager* undoManager);
    void initView(StackView* stackView, TrayView* trayView);

    // 处理备用牌堆点击：翻出顶牌放入手牌区
    bool handleStackClick();

    // 回退翻牌操作
    void undoDraw(const UndoRecord& record);

    void setAnimationLockCallback(std::function<void(bool locked)> callback);

    bool hasCardsRemaining() const;

private:
    GameModel* _gameModel;
    UndoManager* _undoManager;
    StackView* _stackView;
    TrayView* _trayView;
    std::function<void(bool)> _animationLockCallback;
};
```

### 2.18 controllers/GameController.h + .cpp

```cpp
/**
 * 游戏主控制器
 * 管理整个游戏流程：初始化、子控制器协调、胜负判定、Undo
 */
class GameController {
public:
    GameController();
    ~GameController();

    /**
     * 启动游戏
     * @param levelConfigPath 关卡配置 JSON 文件路径
     * @param parentNode 视图挂载的父节点
     */
    void startGame(const std::string& levelConfigPath, cocos2d::Node* parentNode);

    // 处理回退按钮点击
    void handleUndo();

private:
    // --- 数据 ---
    GameModel _gameModel;
    UndoManager _undoManager;

    // --- 子控制器 ---
    PlayFieldController _playFieldController;
    StackController _stackController;

    // --- 视图 ---
    GameView* _gameView;

    // --- 状态 ---
    bool _isAnimating;  // 动画锁，动画期间禁止操作

    void _initModel(const std::string& levelConfigPath);
    void _initViews(cocos2d::Node* parentNode);
    void _initControllers();
    void _bindCallbacks();

    // 动画锁控制
    void _setAnimationLock(bool locked);

    // 胜负检测
    void _checkGameState();
    void _onGameWin();
    void _onGameLose();
};
```

---

## 三、核心交互流程（详细）

### 3.1 游戏初始化

```
GameController::startGame(levelConfigPath, parentNode)
│
├── _initModel(levelConfigPath)
│   ├── LevelConfigLoader::loadFromFile(levelConfigPath) → LevelConfig
│   └── GameModelFromLevelGenerator::generate(config) → _gameModel
│       ├── 分配卡牌 ID
│       ├── _computeCoverRelations() → 计算遮挡关系
│       ├── 设置 faceUp 状态
│       └── Stack[0] → trayCardIds, Stack[1..] → stackCardIds
│
├── _initViews(parentNode)
│   ├── GameView::create() → _gameView
│   ├── parentNode->addChild(_gameView)
│   └── GameView 内部创建 PlayFieldView + StackView + TrayView + UndoButton
│
├── _initControllers()
│   ├── _playFieldController.init(&_gameModel, &_undoManager)
│   ├── _playFieldController.initView(playFieldView, trayView)
│   │   └── 遍历 _gameModel.playfieldCards，为每张牌创建 CardView 并添加到 PlayFieldView
│   ├── _stackController.init(&_gameModel, &_undoManager)
│   └── _stackController.initView(stackView, trayView)
│       └── stackView->initStack(stackCardIds.size())
│
├── 创建初始底牌的 CardView，设置到 TrayView
│
└── _bindCallbacks()
    ├── PlayFieldView::setOnCardClickCallback → _playFieldController.handleCardClick
    ├── StackView::setOnClickCallback → _stackController.handleStackClick
    └── GameView::setOnUndoClickCallback → handleUndo
    （所有回调中先检查 _isAnimating，若为 true 则忽略）
```

### 3.2 点击主牌区卡牌（匹配流程）

```
用户点击主牌区某张牌
│
├── PlayFieldView 触摸事件 → _onCardClickCallback(cardId)
│
├── GameController 检查 _isAnimating → 若 true，return
│
├── PlayFieldController::handleCardClick(cardId)
│   │
│   ├── 获取 CardModel& card = _gameModel->getCard(cardId)
│   ├── 检查 card.faceUp → 若 false，return false
│   │
│   ├── 获取 trayTopId = _gameModel->getTrayTopCardId()
│   ├── MatchRuleService::canMatch(card.face, trayTopCard.face) → 若 false，return false
│   │
│   ├── [记录 Undo]
│   │   _undoManager->recordMatch(cardId, card.position, /*revealedIds稍后填*/, trayTopId)
│   │
│   ├── [更新 Model]
│   │   从 playfieldCards 中标记该牌为已消除（或移除）
│   │   将 cardId 加入 trayCardIds
│   │
│   ├── [触发动画] _animationLockCallback(true)
│   │   CardView 从当前位置飞向 TrayView 位置
│   │   飞行完成后：
│   │   ├── TrayView 设置新顶部牌
│   │   ├── PlayFieldView 移除该 CardView
│   │   │
│   │   ├── [检查遮挡] _revealUncoveredCards(cardId)
│   │   │   遍历 playfieldCards，从每张牌的 coveredByIds 中移除 cardId
│   │   │   若某张牌 coveredByIds 变空 → faceUp = true
│   │   │   对应 CardView 播放翻牌动画
│   │   │   收集被翻开的牌ID → 回填到 UndoRecord.revealedCardIds
│   │   │
│   │   └── _animationLockCallback(false)
│   │
│   └── return true
│
└── GameController::_checkGameState()
    ├── 若 isPlayFieldCleared() → _onGameWin()
    └── 若 !hasMatchableCard() && !hasCardsRemaining() → _onGameLose()
```

### 3.3 点击备用牌堆（翻牌流程）

```
用户点击备用牌堆
│
├── StackView 触摸事件 → _onClickCallback()
│
├── GameController 检查 _isAnimating → 若 true，return
│
├── StackController::handleStackClick()
│   │
│   ├── 检查 stackCardIds 是否为空 → 若空，return false
│   │
│   ├── [记录 Undo]
│   │   int cardId = stackCardIds.back()
│   │   _undoManager->recordDraw(cardId, _gameModel->getTrayTopCardId())
│   │
│   ├── [更新 Model]
│   │   stackCardIds.pop_back()
│   │   trayCardIds.push_back(cardId)
│   │
│   ├── [触发动画] _animationLockCallback(true)
│   │   StackView::popTopCard() → 获取起始世界坐标
│   │   创建该牌的 CardView（正面朝上）
│   │   TrayView::playCardFlyInAnimation(cardView, fromWorldPos)
│   │   动画完成后 → _animationLockCallback(false)
│   │
│   └── return true
```

### 3.4 回退流程

```
用户点击回退按钮
│
├── GameController::handleUndo()
│   │
│   ├── 检查 _isAnimating → 若 true，return
│   ├── _undoManager.undo() → record（若无记录，return）
│   │
│   ├── _setAnimationLock(true)
│   │
│   ├── switch (record.type)
│   │   │
│   │   ├── case MATCH:
│   │   │   _playFieldController.undoMatch(record)
│   │   │   │
│   │   │   ├── [反向翻牌] 将 record.revealedCardIds 中的牌翻回背面
│   │   │   │   对应 CardView::showBack() + 恢复 coveredByIds
│   │   │   │   CardModel.faceUp = false
│   │   │   │
│   │   │   ├── [卡牌飞回] TrayView 移除当前顶部牌
│   │   │   │   CardView 从 TrayView 位置飞回 record.fromPosition
│   │   │   │   飞行完成后 → PlayFieldView 重新添加该 CardView
│   │   │   │
│   │   │   ├── [恢复 Model]
│   │   │   │   从 trayCardIds 移除该牌
│   │   │   │   playfieldCards 中恢复该牌
│   │   │   │   恢复 coveredByIds
│   │   │   │
│   │   │   └── TrayView 恢复为 record.previousTrayTopId 的牌
│   │   │
│   │   └── case DRAW:
│   │       _stackController.undoDraw(record)
│   │       │
│   │       ├── [卡牌飞回] TrayView 移除当前顶部牌
│   │       │   CardView 飞回 StackView 位置 → 变为背面
│   │       │
│   │       ├── [恢复 Model]
│   │       │   trayCardIds.pop_back()
│   │       │   stackCardIds.push_back(cardId)
│   │       │
│   │       └── StackView 恢复一张背面牌
│   │           TrayView 恢复为 record.previousTrayTopId 的牌
│   │
│   └── _setAnimationLock(false)（在最后一个动画的回调中）
│
└── 更新回退按钮状态: setUndoButtonEnabled(_undoManager.canUndo())
```

---

## 四、开发阶段

### Phase 1：基础骨架（目标：编译通过，显示静态画面）

按以下顺序创建文件并实现：

| 序号 | 文件 | 要点 |
|------|------|------|
| 1 | `utils/CardEnums.h` | 枚举定义，纯头文件 |
| 2 | `configs/models/LevelConfig.h` | 纯数据结构，纯头文件 |
| 3 | `configs/models/CardResConfig.h + .cpp` | 静态方法，路径拼接 |
| 4 | `configs/loaders/LevelConfigLoader.h + .cpp` | rapidjson 解析 JSON |
| 5 | `models/CardModel.h` | 纯数据结构 |
| 6 | `models/GameModel.h + .cpp` | getTrayTopCardId, getCard |
| 7 | `services/GameModelFromLevelGenerator.h + .cpp` | 核心：遮挡关系计算 |
| 8 | `views/CardView.h + .cpp` | 核心：精灵拼接布局 |
| 9 | `views/PlayFieldView.h + .cpp` | 容器 + CardView 管理 |
| 10 | `views/StackView.h + .cpp` | 扇形展开布局 |
| 11 | `views/TrayView.h + .cpp` | 单牌显示 |
| 12 | `views/GameView.h + .cpp` | 顶层布局容器 |
| 13 | `controllers/GameController.h + .cpp` | 仅实现 startGame 中的 _initModel + _initViews，不处理交互 |

**验收标准**：运行后看到游戏背景、主牌区若干正面/背面牌、左下备用牌堆（扇形展开背面牌）、右下底牌（一张正面牌）。

### Phase 2：核心玩法（目标：可点击匹配 + 翻牌）

| 序号 | 文件 | 要点 |
|------|------|------|
| 1 | `services/MatchRuleService.h + .cpp` | canMatch 含 K-A 循环 |
| 2 | `controllers/PlayFieldController.h + .cpp` | handleCardClick 全流程 |
| 3 | `controllers/StackController.h + .cpp` | handleStackClick 全流程 |
| 4 | 完善 `CardView` | playFlipToFrontAnimation, playMoveToAnimation |
| 5 | 完善 `GameController` | _initControllers, _bindCallbacks, _checkGameState, _isAnimating 锁 |

**验收标准**：点击主牌区翻开的牌可匹配飞入手牌区，被遮挡牌自动翻开；点击备用牌堆可翻牌；动画期间操作被锁定。

### Phase 3：回退功能

| 序号 | 文件 | 要点 |
|------|------|------|
| 1 | `models/UndoModel.h` | 纯数据结构 |
| 2 | `managers/UndoManager.h + .cpp` | recordMatch, recordDraw, undo |
| 3 | 完善 `PlayFieldController` | undoMatch：恢复遮挡 + 反向飞行 |
| 4 | 完善 `StackController` | undoDraw：牌飞回牌堆 |
| 5 | 完善 `GameController` | handleUndo, 回退按钮绑定 |

**验收标准**：连续操作后多次点击回退，卡牌逐步回到原位，状态完全恢复。无记录时按钮置灰。

### Phase 4：收尾

| 序号 | 内容 |
|------|------|
| 1 | 胜利/失败弹窗 |
| 2 | Model 序列化/反序列化（存档恢复） |
| 3 | 边界情况测试：空牌堆、全部遮挡、连续快速点击 |

---

## 五、核心难点与注意事项

| 难点 | 详细说明 | 建议方案 |
|------|----------|----------|
| **遮挡关系计算** | 双重循环 O(n²)，需正确判断矩形交集和上下层关系。cocos2d 中 y 轴向上，y 越大 = 屏幕越上方 = 视觉上"在上面" = 遮挡下方牌 | 取 card_bg.png 实际尺寸作为卡牌 Rect，用 Rect::intersectsRect 判断交集 |
| **CardView 精灵拼接** | 大数字/小数字/花色的相对定位必须精确，锚点和父节点坐标系要统一 | CardView 的 contentSize = card_bg 尺寸，所有子精灵相对于此定位 |
| **动画与数据一致性** | 动画是异步的，Model 应在动画开始前就更新完毕，动画仅是视觉表现 | 先改 Model → 再播动画 → 动画完成回调中做清理（如 removeChild） |
| **动画锁** | 防止快速连点导致状态错乱 | `_isAnimating` 标志，在所有回调入口处检查 |
| **Undo revealedCardIds** | recordMatch 时还不知道哪些牌会被翻开，需要先记录再回填 | 在 _revealUncoveredCards 执行后，将结果回填到刚 push 的 UndoRecord 中（通过 undoStack.back().revealedCardIds = ...） |
| **StackView 扇形布局** | 每张牌偏移 kStackCardOffsetX，总宽度 = cardWidth + (n-1)*offset，需居中或左对齐 | 建议左对齐，锚点(0, 0.5)，从左到右排列 |
| **坐标系转换** | CardView 从 PlayFieldView 飞向 TrayView，跨越不同父节点 | 使用 `convertToWorldSpace` / `convertToNodeSpace` 转换坐标 |
