#include "TrayView.h"

USING_NS_CC;

TrayView* TrayView::create()
{
    TrayView* view = new (std::nothrow) TrayView();
    if (view && view->init())
    {
        view->autorelease();
        return view;
    }
    CC_SAFE_DELETE(view);
    return nullptr;
}

bool TrayView::init()
{
    if (!Node::init())
        return false;

    _topCardView = nullptr;
    setContentSize(Size(CardView::kCardWidth, CardView::kCardHeight));
    setAnchorPoint(Vec2(0.5f, 0.5f));
    return true;
}

void TrayView::setTopCard(CardView* cardView)
{
    if (!cardView) return;

    // 将旧顶部牌压栈（隐藏但不移除，供 Undo 恢复）
    if (_topCardView)
    {
        _topCardView->setVisible(false);
        _cardStack.push_back(_topCardView);
    }

    _topCardView = cardView;
    cardView->setAnchorPoint(Vec2(0.5f, 0.5f));
    cardView->setPosition(Vec2::ZERO);
    cardView->setVisible(true);

    if (cardView->getParent() != this)
        addChild(cardView, static_cast<int>(_cardStack.size()) + 1);
}

CardView* TrayView::getTopCardView() const
{
    return _topCardView;
}

cocos2d::Vec2 TrayView::getTopCardWorldPosition() const
{
    return convertToWorldSpace(Vec2::ZERO);
}

void TrayView::playCardFlyInAnimation(CardView* cardView,
                                       const Vec2& fromWorldPos,
                                       float duration,
                                       std::function<void()> onComplete)
{
    if (!cardView) return;

    // 先将牌加入此节点，定位到起点（转为本地坐标）
    Vec2 localStart = convertToNodeSpace(fromWorldPos);
    cardView->setAnchorPoint(Vec2(0.5f, 0.5f));
    cardView->setPosition(localStart);
    cardView->setVisible(true);
    cardView->showFront();

    if (cardView->getParent() != this)
        addChild(cardView, static_cast<int>(_cardStack.size()) + 1);

    // 旧顶部牌压栈
    if (_topCardView)
    {
        _topCardView->setVisible(false);
        _cardStack.push_back(_topCardView);
    }
    _topCardView = cardView;

    // 飞行到中心
    auto moveAction = MoveTo::create(duration, Vec2::ZERO);
    auto easeAction = EaseOut::create(moveAction, 2.0f);
    auto notify = CallFunc::create([onComplete]()
    {
        if (onComplete) onComplete();
    });
    cardView->runAction(Sequence::create(easeAction, notify, nullptr));
}

void TrayView::restorePreviousTopCard(CardView* previousCard)
{
    // 移除当前顶部牌
    if (_topCardView)
    {
        _topCardView->removeFromParent();
        _topCardView = nullptr;
    }

    // 从栈中找到 previousCard 并恢复（若传入为栈顶）
    if (!_cardStack.empty() && _cardStack.back() == previousCard)
    {
        _cardStack.pop_back();
        _topCardView = previousCard;
        previousCard->setVisible(true);
    }
    else if (previousCard)
    {
        // 直接设置（初始底牌的 Undo 场景）
        _topCardView = previousCard;
        previousCard->setVisible(true);
        if (previousCard->getParent() != this)
            addChild(previousCard, 0);
    }
}

CardView* TrayView::detachTopCard()
{
    CardView* top = _topCardView;
    _topCardView = nullptr;

    // 恢复栈中上一张为顶部牌
    if (!_cardStack.empty())
    {
        _topCardView = _cardStack.back();
        _cardStack.pop_back();
        _topCardView->setVisible(true);
    }

    return top;
}
