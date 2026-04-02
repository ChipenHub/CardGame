#include "StackController.h"
#include "views/CardView.h"

USING_NS_CC;

void StackController::init(GameModel* gameModel, UndoManager* undoManager)
{
    _gameModel   = gameModel;
    _undoManager = undoManager;
}

void StackController::initView(StackView* stackView, TrayView* trayView)
{
    _stackView = stackView;
    _trayView  = trayView;
}

bool StackController::handleStackClick()
{
    if (!_gameModel || !_stackView || !_trayView) return false;
    if (_gameModel->stackCardIds.empty()) return false;

    int cardId    = _gameModel->stackCardIds.back();
    int prevTrayTop = _gameModel->getTrayTopCardId();

    // --- 记录 Undo ---
    _undoManager->recordDraw(cardId, prevTrayTop);

    // --- 更新 Model ---
    _gameModel->stackCardIds.pop_back();
    _gameModel->trayCardIds.push_back(cardId);

    CardModel& card = _gameModel->getCard(cardId);
    card.faceUp = true;

    // --- 触发动画 ---
    if (_animationLockCallback) _animationLockCallback(true);

    Vec2 stackWorldPos = _stackView->popTopCard();

    CardView* cv = CardView::create(card.id, card.face, card.suit);
    if (!cv)
    {
        if (_animationLockCallback) _animationLockCallback(false);
        return false;
    }

    _trayView->playCardFlyInAnimation(cv, stackWorldPos, 0.3f, [this]()
    {
        if (_animationLockCallback) _animationLockCallback(false);
    });

    return true;
}

void StackController::undoDraw(const UndoRecord& record)
{
    if (!_gameModel || !_stackView || !_trayView) return;

    // 1. 从手牌区顶部移除牌视图
    CardView* cv = _trayView->detachTopCard();
    if (cv) cv->removeFromParent();

    // 2. 恢复模型
    auto& tray = _gameModel->trayCardIds;
    tray.erase(std::remove(tray.begin(), tray.end(), record.cardId), tray.end());

    _gameModel->stackCardIds.push_back(record.cardId);
    _gameModel->getCard(record.cardId).faceUp = false;

    // 3. 恢复 StackView 视觉（补回一张背面牌）
    _stackView->pushBackCard();
}

void StackController::setAnimationLockCallback(std::function<void(bool)> callback)
{
    _animationLockCallback = callback;
}

bool StackController::hasCardsRemaining() const
{
    return _gameModel && !_gameModel->stackCardIds.empty();
}
