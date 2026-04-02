#ifndef __PLAY_FIELD_CONTROLLER_H__
#define __PLAY_FIELD_CONTROLLER_H__

#include "cocos2d.h"
#include "models/GameModel.h"
#include "models/UndoModel.h"
#include "managers/UndoManager.h"
#include "views/PlayFieldView.h"
#include "views/TrayView.h"
#include <functional>
#include <vector>

/**
 * PlayFieldController
 * 主牌区控制器
 *
 * 职责：
 *   - 初始化主牌区视图（创建 CardView 并添加到 PlayFieldView）
 *   - 响应卡牌点击：匹配判断 → 数据更新 → 飞行动画 → 翻牌动画 → Undo 回填
 *   - 提供 undoMatch() 供 GameController 在 Undo 时调用
 *
 * 扩展设计：
 *   匹配判断走 MatchRuleService（含 CardAbilityRegistry 覆盖检查）。
 *   匹配成功后通过 CardAbilityRegistry 查询额外消除效果。
 *   新功能牌只需实现 ICardAbility 并注册，无需改此类。
 */
class PlayFieldController
{
public:
    PlayFieldController() = default;

    /**
     * 绑定数据层
     * @param gameModel   GameModel 指针（由 GameController 持有）
     * @param undoManager UndoManager 指针（由 GameController 持有）
     */
    void init(GameModel* gameModel, UndoManager* undoManager);

    /**
     * 初始化视图：遍历 gameModel.playfieldCards，创建 CardView 添加到 playFieldView
     * 并绑定点击回调
     */
    void initView(PlayFieldView* playFieldView, TrayView* trayView);

    /**
     * 处理主牌区卡牌点击
     * @return true 如果匹配成功并触发动画
     */
    bool handleCardClick(int cardId);

    /**
     * 回退匹配操作（由 GameController::handleUndo 调用）
     * Phase 2 实现基础 Undo：将牌从手牌区移回主牌区，重新遮挡之前翻开的牌
     */
    void undoMatch(const UndoRecord& record);

    /**
     * 设置动画锁回调（true=加锁, false=解锁）
     * 由 GameController 提供，动画期间屏蔽所有点击
     */
    void setAnimationLockCallback(std::function<void(bool)> callback);

    /** 主牌区是否已清空（胜利条件） */
    bool isPlayFieldCleared() const;

    /**
     * 是否存在可匹配的牌（手牌区有顶牌时调用）
     * 用于判断是否触发败局
     */
    bool hasMatchableCard() const;

private:
    GameModel*    _gameModel    = nullptr;
    UndoManager*  _undoManager  = nullptr;
    PlayFieldView* _playFieldView = nullptr;
    TrayView*      _trayView      = nullptr;
    std::function<void(bool)> _animationLockCallback;

    /**
     * 消除 removedCardId 后，从所有牌的 coveredByIds 中移除该 ID；
     * 若某牌 coveredByIds 变为空，翻开该牌并触发翻牌动画。
     * @return 被翻开的牌 ID 列表
     */
    std::vector<int> _revealUncoveredCards(int removedCardId);

    /**
     * 处理功能牌能力效果：查询 CardAbilityRegistry，调用 onRemoved()，
     * 将额外消除的牌从模型和视图中移除，并级联触发遮挡翻开。
     * @return 额外被移除的牌 ID 列表
     */
    std::vector<int> _applyAbilityEffects(int cardId);
};

#endif // __PLAY_FIELD_CONTROLLER_H__
