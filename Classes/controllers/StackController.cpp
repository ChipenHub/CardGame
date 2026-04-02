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

    int cardId      = _gameModel->stackCardIds.back();
    int prevTrayTop = _gameModel->getTrayTopCardId();

    // --- 记录 Undo ---
    _undoManager->recordDraw(cardId, prevTrayTop);

    // --- 更新 Model ---
    _gameModel->stackCardIds.pop_back();
    _gameModel->trayCardIds.push_back(cardId);

    CardModel& card = _gameModel->getCard(cardId);
    card.faceUp = true;

    // --- 动画：从堆顶弹出 + 飞行中翻牌 ---
    if (_animationLockCallback) _animationLockCallback(true);

    // 从 StackView 弹出最右侧（堆顶）的背面牌，返回其世界坐标
    Vec2 stackWorldPos = _stackView->popTopCard();

    CardView* cv = CardView::create(card.id, card.face, card.suit);
    if (!cv)
    {
        if (_animationLockCallback) _animationLockCallback(false);
        return false;
    }
    // cv 默认显示背面

    // 动画挂载层：StackView / TrayView 的共同父节点（GameView）
    Node* animLayer = _stackView->getParent();
    if (!animLayer) animLayer = _stackView;

    Vec2 trayWorldPos = _trayView->getTopCardWorldPosition();

    // 起点 Y 强制与手牌区齐平，实现水平飞行而非斜线
    Vec2 startLocal = animLayer->convertToNodeSpace(Vec2(stackWorldPos.x, trayWorldPos.y));
    Vec2 endLocal   = animLayer->convertToNodeSpace(trayWorldPos);
    Vec2 midLocal   = (startLocal + endLocal) * 0.5f;

    cv->setAnchorPoint(Vec2(0.5f, 0.5f));
    cv->setPosition(startLocal);
    animLayer->addChild(cv, 100); // 高 z，飞行时置顶

    static const float kHalf = 0.18f; // 单程时长

    // 前半段：向中点移动 + X 轴缩小到 0（背面消失）
    auto move1    = MoveTo::create(kHalf, midLocal);
    auto scaleOut = ScaleTo::create(kHalf, 0.0f, 1.0f);
    auto phase1   = Spawn::create(move1, scaleOut, nullptr);

    // 中点：切换为正面
    auto flipCall = CallFunc::create([cv]() { cv->showFront(); });

    // 后半段：向终点移动 + X 轴展开到 1（正面出现）
    auto move2   = MoveTo::create(kHalf, endLocal);
    auto scaleIn = ScaleTo::create(kHalf, 1.0f, 1.0f);
    auto phase2  = Spawn::create(move2, scaleIn, nullptr);

    // 落地：移交给 TrayView
    auto done = CallFunc::create([this, cv]()
    {
        cv->retain();
        cv->removeFromParent();
        _trayView->setTopCard(cv);
        cv->release();

        if (_animationLockCallback) _animationLockCallback(false);
    });

    cv->runAction(Sequence::create(phase1, flipCall, phase2, done, nullptr));
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

    // 3. 恢复 StackView 视觉（补回一张背面牌到堆顶）
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
