#include "PlayFieldController.h"
#include "services/MatchRuleService.h"
#include "services/CardAbilityRegistry.h"
#include "services/AudioService.h"
#include "views/CardView.h"
#include "configs/models/CardResConfig.h"
#include <algorithm>

USING_NS_CC;

void PlayFieldController::init(GameModel* gameModel, UndoManager* undoManager)
{
    _gameModel   = gameModel;
    _undoManager = undoManager;
}

void PlayFieldController::initView(PlayFieldView* playFieldView, TrayView* trayView)
{
    _playFieldView = playFieldView;
    _trayView      = trayView;

    for (int cardId : _gameModel->playfieldCardIds)
    {
        const CardModel& card = _gameModel->getCard(cardId);
        CardView* cv = CardView::create(card.id, card.face, card.suit);
        if (!cv) continue;

        card.faceUp ? cv->showFront() : cv->showBack();
        _playFieldView->addCardView(cv, Vec2(card.position.x, card.position.y), card.id);
    }
}

bool PlayFieldController::handleCardClick(int cardId)
{
    if (!_gameModel || !_playFieldView || !_trayView) return false;

    CardModel& card = _gameModel->getCard(cardId);
    if (!card.faceUp) return false;

    int trayTopId = _gameModel->getTrayTopCardId();
    if (trayTopId == -1) return false;

    const CardModel& trayCard = _gameModel->getCard(trayTopId);
    if (!MatchRuleService::canMatch(card, trayCard)) return false;

    // --- 记录 Undo（revealedCardIds 在动画后回填） ---
    _undoManager->recordMatch(cardId, card.position, trayTopId);

    // --- 更新 Model ---
    auto& pfIds = _gameModel->playfieldCardIds;
    pfIds.erase(std::remove(pfIds.begin(), pfIds.end(), cardId), pfIds.end());
    _gameModel->trayCardIds.push_back(cardId);

    // --- 触发飞行动画 ---
    if (_animationLockCallback) _animationLockCallback(true);

    CardView* cv = _playFieldView->getCardView(cardId);
    if (!cv) return false;

    // 世界坐标必须在 removeFromParent 之前获取，此时父节点关系还在
    Vec2 cardWorldPos = cv->getParent()
        ? cv->getParent()->convertToWorldSpace(cv->getPosition())
        : cv->getPosition();

    // retain 必须在 removeCardView（内部 removeFromParent）之前调用，
    // 否则引用计数归零节点被释放，cv 变成悬空指针
    cv->retain();
    _playFieldView->removeCardView(cardId);

    // 挂到动画层（PlayFieldView 的父节点），保证飞行时在所有牌上方
    Node* animLayer = _playFieldView->getParent();
    if (!animLayer) animLayer = _playFieldView;

    cv->setPosition(animLayer->convertToNodeSpace(cardWorldPos));
    animLayer->addChild(cv, 100);
    cv->release(); // addChild 已 retain，平衡上面的 retain

    Vec2 trayWorldPos = _trayView->getTopCardWorldPosition();

    cv->playMoveToAnimation(trayWorldPos, 0.3f, [this, cv, cardId, trayWorldPos, animLayer]()
    {
        // 动画完成：将牌交给 TrayView
        AudioService::playClear();
        cv->removeFromParent();
        _trayView->setTopCard(cv);

        // 检查能力效果（功能牌）
        std::vector<int> extraRemoved = _applyAbilityEffects(cardId);
        if (!extraRemoved.empty())
            _undoManager->backFillExtraRemovedIds(extraRemoved);

        // 翻开不再被遮挡的牌
        std::vector<int> revealed = _revealUncoveredCards(cardId);
        for (int extraId : extraRemoved)
        {
            auto moreRevealed = _revealUncoveredCards(extraId);
            revealed.insert(revealed.end(), moreRevealed.begin(), moreRevealed.end());
        }
        _undoManager->backFillRevealedIds(revealed);

        if (_animationLockCallback) _animationLockCallback(false);
    });

    return true;
}

std::vector<int> PlayFieldController::_revealUncoveredCards(int removedCardId)
{
    std::vector<int> revealed;

    for (int id : _gameModel->playfieldCardIds)
    {
        CardModel& c = _gameModel->getCard(id);
        auto& cov = c.coveredByIds;
        auto it = std::find(cov.begin(), cov.end(), removedCardId);
        if (it == cov.end()) continue;

        cov.erase(it);

        if (cov.empty() && !c.faceUp)
        {
            c.faceUp = true;
            revealed.push_back(id);

            CardView* cv = _playFieldView->getCardView(id);
            if (cv) cv->playFlipToFrontAnimation(0.3f, nullptr);
        }
    }

    return revealed;
}

std::vector<int> PlayFieldController::_applyAbilityEffects(int cardId)
{
    const CardModel& card = _gameModel->getCard(cardId);
    ICardAbility* ability = CardAbilityRegistry::getInstance().getAbility(card.face, card.suit);
    if (!ability) return {};

    std::vector<int> extraIds = ability->onRemoved(cardId, *_gameModel);

    for (int extraId : extraIds)
    {
        // 更新模型
        auto& pfIds = _gameModel->playfieldCardIds;
        pfIds.erase(std::remove(pfIds.begin(), pfIds.end(), extraId), pfIds.end());

        // 移除视图（直接消除，无飞行动画；如需动画可在 ICardAbility 中自行处理）
        _playFieldView->removeCardView(extraId);
    }

    return extraIds;
}

void PlayFieldController::undoMatch(const UndoRecord& record)
{
    if (!_gameModel || !_playFieldView || !_trayView) return;

    // 1. 从手牌区顶部取回牌
    CardView* cv = _trayView->detachTopCard();
    if (!cv) return;

    // 2. 恢复模型：从 trayCardIds 移除，重新加入 playfieldCardIds
    auto& tray = _gameModel->trayCardIds;
    tray.erase(std::remove(tray.begin(), tray.end(), record.cardId), tray.end());

    _gameModel->playfieldCardIds.push_back(record.cardId);

    // 3. 恢复 coveredByIds：对之前被翻开的牌重新遮挡
    CardModel& restoredCard = _gameModel->getCard(record.cardId);
    for (int revealedId : record.revealedCardIds)
    {
        CardModel& revCard = _gameModel->getCard(revealedId);
        revCard.coveredByIds.push_back(record.cardId);
        revCard.faceUp = false;

        CardView* revCv = _playFieldView->getCardView(revealedId);
        if (revCv) revCv->playFlipToBackAnimation(0.3f, nullptr);
    }

    // 4. 将牌飞回原位
    restoredCard.faceUp = true;

    Vec2 worldTarget = _playFieldView->convertToWorldSpace(
        Vec2(restoredCard.position.x, restoredCard.position.y));

    // 先把 cv 从 TrayView 安全摘下（detachTopCard 不调 removeFromParent）
    Node* animLayer = _playFieldView->getParent();
    if (!animLayer) animLayer = _playFieldView;

    Vec2 trayWorldPos = _trayView->getTopCardWorldPosition();
    cv->retain();           // 防止 removeFromParent 后引用计数归零
    cv->removeFromParent(); // 真正从 TrayView 断开父子关系
    cv->setPosition(animLayer->convertToNodeSpace(trayWorldPos));
    animLayer->addChild(cv, 100); // addChild 会再次 retain
    cv->release();          // 平衡上面的手动 retain

    // 按值捕获 cardId，避免捕获 record 引用（record 是栈上局部变量，回调时已失效）
    int capturedCardId = record.cardId;
    cv->playMoveToAnimation(worldTarget, 0.3f, [this, cv, capturedCardId, worldTarget]()
    {
        cv->removeFromParent();
        cv->showFront();
        _playFieldView->addCardView(
            cv,
            Vec2(_gameModel->getCard(capturedCardId).position.x,
                 _gameModel->getCard(capturedCardId).position.y),
            capturedCardId);
    });
}

void PlayFieldController::setAnimationLockCallback(std::function<void(bool)> callback)
{
    _animationLockCallback = callback;
}

bool PlayFieldController::isPlayFieldCleared() const
{
    return _gameModel && _gameModel->playfieldCardIds.empty();
}

bool PlayFieldController::hasMatchableCard() const
{
    if (!_gameModel) return false;

    int trayTopId = _gameModel->getTrayTopCardId();
    if (trayTopId == -1) return false;

    const CardModel& trayCard = _gameModel->getCard(trayTopId);

    for (int id : _gameModel->playfieldCardIds)
    {
        const CardModel& c = _gameModel->getCard(id);
        if (c.faceUp && MatchRuleService::canMatch(c, trayCard))
            return true;
    }
    return false;
}
