#ifndef __GAME_CONTROLLER_H__
#define __GAME_CONTROLLER_H__

#include "cocos2d.h"
#include "models/GameModel.h"
#include "managers/UndoManager.h"
#include "controllers/PlayFieldController.h"
#include "controllers/StackController.h"
#include "views/GameView.h"

/**
 * GameController
 * 游戏主控制器
 *
 * 职责：
 *   - 加载关卡 → 生成 GameModel
 *   - 创建 GameView，协调子控制器初始化
 *   - 绑定各区域回调（点击主牌区 / 备用牌堆 / Undo 按钮）
 *   - 维护动画锁 _isAnimating，期间屏蔽所有交互
 *   - 胜负检测
 *   - 协调 Undo 流程（分发给对应子控制器）
 */
class GameController
{
public:
    GameController();
    ~GameController();

    /**
     * 启动游戏
     * @param levelConfigPath 关卡 JSON 路径（相对于 Resources）
     * @param parentNode      GameView 挂载的父节点
     */
    void startGame(const std::string& levelConfigPath, cocos2d::Node* parentNode);

    /** 处理 Undo 按钮点击 */
    void handleUndo();

private:
    // --- 数据 ---
    GameModel        _gameModel;
    UndoManager      _undoManager;

    // --- 子控制器 ---
    PlayFieldController _playFieldController;
    StackController     _stackController;

    // --- 视图 ---
    GameView*             _gameView;
    cocos2d::Node*        _statusSprite; // 胜/败提示图片，有操作时清除

    // --- 状态 ---
    bool _isAnimating;

    // --- 初始化流程 ---
    void _initModel(const std::string& levelConfigPath);
    void _initViews(cocos2d::Node* parentNode);
    void _initControllers();
    void _bindCallbacks();

    // --- 动画锁 ---
    void _setAnimationLock(bool locked);

    // --- 胜负检测（在动画解锁后调用） ---
    void _checkGameState();
    void _onGameWin();
    void _onGameLose();
};

#endif // __GAME_CONTROLLER_H__
