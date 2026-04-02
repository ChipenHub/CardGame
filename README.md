# CardGame

一款基于 **Cocos2d-x 3.x** 开发的单人纸牌消除游戏。玩家通过将主牌区的牌与手牌区顶牌进行点数匹配来消除主牌区所有牌，完成关卡。

---

## 游戏规则

- **匹配条件**：点数差值为 1 即可匹配（无花色限制）；K 与 A 可循环匹配
- **主牌区**：有遮挡关系，被上层牌覆盖的牌呈背面，无法选择；移除上层牌后自动翻面
- **备用牌堆**：点击可依次翻出牌到手牌区，改变手牌区顶牌
- **手牌区**：始终显示一张正面牌，点击主牌区可匹配牌消除它并成为新顶牌
- **Undo**：撤销最近一步操作（匹配消除或翻牌），支持多步
- **胜利**：主牌区所有牌消除完毕
- **失败**：备用牌堆为空且主牌区无任何可匹配牌

---

## 项目亮点

### 架构设计
- **MVC 分层**：`models/`、`views/`、`controllers/` 严格分离，数据与表现零耦合
- **扩展接口 `ICardAbility`**：新增功能牌只需实现接口并注册到 `CardAbilityRegistry`，无需改动任何 Controller / Service 代码
- **`MatchRuleService`** 统一匹配逻辑，支持通过能力注册表覆盖标准规则（如 Joker 万能匹配）
- **`AudioService`** 统一音频入口，所有音效一处管理

### 游戏功能
- 完整的 **Undo 系统**：MATCH / DRAW 两种操作均可回退，含遮挡关系的完整恢复
- **动态遮挡计算**：基于 y 坐标与矩形相交的精确遮挡算法，正确处理多牌叠压场景
- 关卡配置由 **JSON 文件**驱动，新增关卡无需重新编译

### 动画
- 主牌区消除：卡牌飞行动画（EaseOut）
- 备用牌堆翻牌：飞行途中 X 轴翻转，落地时正好完成翻面
- 牌面翻转：X 轴缩放模拟 3D 翻牌效果
- 状态提示（胜/负）：弹性弹出动画
- Undo 按钮点击：跳动反馈动画

---

## 目录结构

```
CardGame/
├── Classes/
│   ├── configs/          # 关卡配置数据结构与加载器
│   ├── controllers/      # GameController / PlayFieldController / StackController
│   ├── managers/         # UndoManager
│   ├── models/           # CardModel / GameModel / UndoModel
│   ├── services/         # MatchRuleService / CardAbilityRegistry / AudioService / ICardAbility
│   ├── utils/            # CardEnums
│   └── views/            # CardView / PlayFieldView / StackView / TrayView / GameView
├── Resources/
│   ├── audio/            # bgm.mp3 / click.mp3 / clear.mp3 / win.mp3 / lose.mp3
│   ├── levels/           # level1.json ...
│   └── views/
│       ├── number/       # 牌面数字图片
│       ├── suits/        # 花色图片
│       └── ui/           # card_bg / card_covered / game_bg / undo / success / no_more_moves
├── cocos2d/              # Cocos2d-x 引擎（子模块）
└── CMakeLists.txt
```

---

## 编译环境要求

| 依赖 | 版本要求 |
|------|----------|
| CMake | ≥ 3.6 |
| Python | ≥ 2.7（Cocos2d-x 工具链依赖） |
| **Windows** | Visual Studio 2019 / 2022，含 C++ 桌面开发组件 |
| **macOS** | Xcode ≥ 10，Command Line Tools |
| **Linux** | GCC ≥ 7，libgl1-mesa-dev，libglu1-mesa-dev |

> C++ 标准：C++11（由 Cocos2d-x 引擎统一设定）

---

## 如何编译

### Windows

```bash
# 1. 克隆仓库（含子模块）
git clone --recurse-submodules <repo-url>
cd CardGame

# 2. 生成 Visual Studio 工程
cmake -B build -G "Visual Studio 17 2022" -A Win32

# 3. 编译（Debug）
cmake --build build --config Debug

# 4. 编译（Release）
cmake --build build --config Release
```

> 也可以用 Visual Studio 直接打开 `build/CardGame.sln` 后编译运行。

### macOS

```bash
cmake -B build -G Xcode
cmake --build build --config Debug
```

### Linux

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

---

## 如何运行

编译完成后，可执行文件位于：

```
build/bin/CardGame/Debug/CardGame.exe        # Windows Debug
build/bin/CardGame/Release/CardGame.exe      # Windows Release
build/bin/CardGame/Debug/CardGame            # macOS / Linux
```

Resources 目录会在编译后自动复制到可执行文件同级目录，直接双击或在终端运行即可。

---

## 关卡配置格式

关卡文件位于 `Resources/levels/`，JSON 格式示例：

```json
{
  "Playfield": [
    { "CardFace": "ACE",  "CardSuit": "CLUBS",    "Position": { "x": 100, "y": 200 } },
    { "CardFace": "TWO",  "CardSuit": "HEARTS",   "Position": { "x": 200, "y": 200 } }
  ],
  "Stack": [
    { "CardFace": "KING", "CardSuit": "SPADES"  },
    { "CardFace": "QUEEN","CardSuit": "DIAMONDS" }
  ]
}
```

- `Playfield`：主牌区的牌及其坐标，坐标为牌左下角像素位置
- `Stack`：备用牌堆，**第一张为开局手牌**，后续按顺序依次翻出

---

## 添加新功能牌

1. 在 `Classes/services/` 新建能力类，继承 `ICardAbility`：

```cpp
class WildAbility : public ICardAbility {
    bool canMatchWithAny() const override { return true; }
};
```

2. 在游戏启动时注册：

```cpp
CardAbilityRegistry::getInstance().registerAbility(
    CFT_JOKER, CST_NONE, std::make_unique<WildAbility>());
```

无需修改任何现有逻辑。
