#include "UndoManager.h"

void UndoManager::recordMatch(int cardId,
                               const cocos2d::Vec2& fromPosition,
                               int previousTrayTopId)
{
    UndoRecord rec;
    rec.type              = UndoRecord::Type::MATCH;
    rec.cardId            = cardId;
    rec.fromPosition      = fromPosition;
    rec.previousTrayTopId = previousTrayTopId;
    _model.pushRecord(rec);
}

void UndoManager::recordDraw(int cardId, int previousTrayTopId)
{
    UndoRecord rec;
    rec.type              = UndoRecord::Type::DRAW;
    rec.cardId            = cardId;
    rec.fromPosition      = cocos2d::Vec2::ZERO;
    rec.previousTrayTopId = previousTrayTopId;
    _model.pushRecord(rec);
}

void UndoManager::backFillRevealedIds(const std::vector<int>& ids)
{
    _model.backFillRevealedIds(ids);
}

void UndoManager::backFillExtraRemovedIds(const std::vector<int>& ids)
{
    _model.backFillExtraRemovedIds(ids);
}

UndoRecord UndoManager::undo()
{
    return _model.popRecord();
}

bool UndoManager::canUndo() const
{
    return _model.canUndo();
}

void UndoManager::clear()
{
    _model.clear();
}
