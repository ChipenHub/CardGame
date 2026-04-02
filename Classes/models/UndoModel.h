#ifndef __UNDO_MODEL_H__
#define __UNDO_MODEL_H__

#include "cocos2d.h"
#include <vector>

/**
 * UndoRecord
 * 记录一次可撤销操作的完整数据快照
 *
 * MATCH：主牌区卡牌与手牌区顶牌匹配消除
 * DRAW ：从备用牌堆翻出一张牌到手牌区
 *
 * extraRemovedCardIds：功能牌能力额外消除的牌（普通牌此字段为空）
 *   — 为将来功能牌 Undo 预留，Phase 2 暂不实现 Undo 还原能力效果
 */
struct UndoRecord
{
    enum class Type { MATCH, DRAW };

    Type                type;
    int                 cardId;             // 操作主牌 ID
    cocos2d::Vec2       fromPosition;       // 牌在主牌区的原始坐标（MATCH 时有效）
    std::vector<int>    revealedCardIds;    // 本次操作翻开的牌 ID（MATCH 时动画后回填）
    std::vector<int>    extraRemovedCardIds;// 功能牌能力额外移除的牌 ID
    int                 previousTrayTopId; // 操作前手牌区顶部牌 ID
};

/**
 * UndoModel
 * 撤销记录栈（数据层），由 UndoManager 持有
 */
struct UndoModel
{
    std::vector<UndoRecord> undoStack;

    bool canUndo() const { return !undoStack.empty(); }

    void pushRecord(const UndoRecord& record) { undoStack.push_back(record); }

    UndoRecord popRecord()
    {
        UndoRecord rec = undoStack.back();
        undoStack.pop_back();
        return rec;
    }

    /** 回填最近一条记录的 revealedCardIds（飞行动画完成后调用） */
    void backFillRevealedIds(const std::vector<int>& ids)
    {
        if (!undoStack.empty())
            undoStack.back().revealedCardIds = ids;
    }

    /** 回填最近一条记录的 extraRemovedCardIds */
    void backFillExtraRemovedIds(const std::vector<int>& ids)
    {
        if (!undoStack.empty())
            undoStack.back().extraRemovedCardIds = ids;
    }

    void clear() { undoStack.clear(); }
};

#endif // __UNDO_MODEL_H__
