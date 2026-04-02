#ifndef __UNDO_MANAGER_H__
#define __UNDO_MANAGER_H__

#include "models/UndoModel.h"
#include <vector>

/**
 * UndoManager
 * 撤销管理器，持有 UndoModel 并提供语义化操作接口
 * 作为 GameController 的成员（非单例），随关卡生命周期存在
 */
class UndoManager
{
public:
    UndoManager() = default;

    // 记录一次匹配消除操作
    // revealedCardIds 和 extraRemovedCardIds 可在动画完成后通过 backFill* 回填
    void recordMatch(int cardId,
                     const cocos2d::Vec2& fromPosition,
                     int previousTrayTopId);

    // 记录一次翻牌操作
    void recordDraw(int cardId, int previousTrayTopId);

    // 动画完成后回填翻开的牌 ID（MATCH 操作使用）
    void backFillRevealedIds(const std::vector<int>& ids);

    // 动画完成后回填功能牌额外消除的牌 ID
    void backFillExtraRemovedIds(const std::vector<int>& ids);

    // 弹出并返回最近的撤销记录（调用前必须先检查 canUndo()）
    UndoRecord undo();

    bool canUndo() const;
    void clear();

private:
    UndoModel _model;
};

#endif // __UNDO_MANAGER_H__
