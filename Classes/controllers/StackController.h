#ifndef __STACK_CONTROLLER_H__
#define __STACK_CONTROLLER_H__

#include "cocos2d.h"
#include "models/GameModel.h"
#include "models/UndoModel.h"
#include "managers/UndoManager.h"
#include "views/StackView.h"
#include "views/TrayView.h"
#include <functional>

/**
 * StackController
 * 备用牌堆控制器
 *
 * 职责：
 *   - 响应备用牌堆点击：从 stackCardIds 顶部取牌 → 更新模型 → 动画飞入手牌区
 *   - 提供 undoDraw() 供 GameController 在 Undo 时调用
 */
class StackController
{
public:
    StackController() = default;

    void init(GameModel* gameModel, UndoManager* undoManager);
    void initView(StackView* stackView, TrayView* trayView);

    /**
     * 处理备用牌堆点击
     * @return true 如果成功翻出一张牌
     */
    bool handleStackClick();

    /**
     * 回退翻牌操作（由 GameController::handleUndo 调用）
     */
    void undoDraw(const UndoRecord& record);

    void setAnimationLockCallback(std::function<void(bool)> callback);

    bool hasCardsRemaining() const;

private:
    GameModel*   _gameModel   = nullptr;
    UndoManager* _undoManager = nullptr;
    StackView*   _stackView   = nullptr;
    TrayView*    _trayView    = nullptr;
    std::function<void(bool)> _animationLockCallback;
};

#endif // __STACK_CONTROLLER_H__
