#include "PlayFieldView.h"

USING_NS_CC;

PlayFieldView* PlayFieldView::create()
{
    PlayFieldView* view = new (std::nothrow) PlayFieldView();
    if (view && view->init())
    {
        view->autorelease();
        return view;
    }
    CC_SAFE_DELETE(view);
    return nullptr;
}

bool PlayFieldView::init()
{
    if (!Node::init())
        return false;

    setContentSize(Size(1080.0f, 1500.0f));
    setAnchorPoint(Vec2::ZERO);
    return true;
}

void PlayFieldView::addCardView(CardView* cardView, const Vec2& position, int zOrder /*= 1*/)
{
    if (!cardView) return;

    cardView->setPosition(position);

    // 将 Controller 设置的全局回调转发给每张牌
    cardView->setClickCallback([this](int cardId)
    {
        if (_onCardClickCallback)
            _onCardClickCallback(cardId);
    });

    _cardViews[cardView->getCardId()] = cardView;
    addChild(cardView, zOrder);
}

void PlayFieldView::removeCardView(int cardId)
{
    auto it = _cardViews.find(cardId);
    if (it != _cardViews.end())
    {
        it->second->removeFromParent();
        _cardViews.erase(it);
    }
}

CardView* PlayFieldView::getCardView(int cardId) const
{
    auto it = _cardViews.find(cardId);
    return (it != _cardViews.end()) ? it->second : nullptr;
}

void PlayFieldView::setOnCardClickCallback(std::function<void(int)> callback)
{
    _onCardClickCallback = callback;

    // 更新已有卡牌的回调
    for (auto& kv : _cardViews)
    {
        kv.second->setClickCallback([this](int cardId)
        {
            if (_onCardClickCallback)
                _onCardClickCallback(cardId);
        });
    }
}
