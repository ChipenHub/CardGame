#ifndef __GAME_CONTROLLER_H__
#define __GAME_CONTROLLER_H__

#include "cocos2d.h"
#include "models/GameModel.h"
#include "views/GameView.h"

/**
 * GameController
 * 游戏主控制器（Phase 1 最小实现）
 *
 * 职责：
 *   - 加载关卡配置，生成 GameModel
 *   - 创建 GameView 并挂载到父节点
 *   - 根据 GameModel 初始化各区域的视图（CardView 布局）
 *
 * Phase 2 起补充：子控制器、交互绑定、胜负判定
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

private:
    GameModel   _gameModel;
    GameView*   _gameView;

    /** 从配置文件加载并生成 GameModel */
    void _initModel(const std::string& levelConfigPath);

    /** 创建 GameView 并挂载 */
    void _initView(cocos2d::Node* parentNode);

    /**
     * 根据 GameModel 填充视图：
     *   - 主牌区每张牌创建 CardView，按 position 放置
     *   - 备用牌堆初始化 StackView
     *   - 初始底牌创建 CardView 并交给 TrayView
     */
    void _populateViews();
};

#endif // __GAME_CONTROLLER_H__
