#include "StackView.h"
#include "CardView.h"
#include "configs/models/CardResConfig.h"

USING_NS_CC;

const float StackView::kStackCardOffsetX = 18.0f;

StackView* StackView::create()
{
    StackView* view = new (std::nothrow) StackView();
    if (view && view->init())
    {
        view->autorelease();
        return view;
    }
    CC_SAFE_DELETE(view);
    return nullptr;
}

bool StackView::init()
{
    if (!Node::init())
        return false;

    setAnchorPoint(Vec2(0.5f, 0.5f));
    _setupTouchListener();
    return true;
}

void StackView::initStack(int cardCount)
{
    // 清除旧牌
    for (auto* s : _stackCards)
        s->removeFromParent();
    _stackCards.clear();

    for (int i = 0; i < cardCount; ++i)
    {
        Sprite* card = Sprite::create(CardResConfig::getCoveredPath());
        if (card)
        {
            card->setAnchorPoint(Vec2(0.5f, 0.5f));
            addChild(card, i);
            _stackCards.push_back(card);
        }
    }

    _relayout();
}

void StackView::_relayout()
{
    // 居中排列：可视中心在 x=0，spread 总偏移 = (n-1)*offset
    // 第一张 startX = -(n-1)*offset/2，最后一张 = +(n-1)*offset/2
    int n = static_cast<int>(_stackCards.size());
    float spreadWidth = (n > 1) ? (n - 1) * kStackCardOffsetX : 0.0f;
    float startX = -spreadWidth * 0.5f;
    for (int i = 0; i < n; ++i)
        _stackCards[i]->setPosition(Vec2(startX + i * kStackCardOffsetX, 0.0f));

    // 更新内容尺寸
    float totalWidth = CardView::kCardWidth
                       + (_stackCards.empty() ? 0 : (_stackCards.size() - 1) * kStackCardOffsetX);
    setContentSize(Size(totalWidth, CardView::kCardHeight));
}

cocos2d::Vec2 StackView::popTopCard()
{
    if (_stackCards.empty())
        return Vec2::ZERO;

    Sprite* top = _stackCards.back();
    Vec2 worldPos = top->convertToWorldSpace(Vec2::ZERO);
    // 补偿锚点：Sprite 锚点 (0.5,0.5) 对应中心
    worldPos = top->getParent()->convertToWorldSpace(top->getPosition());

    top->removeFromParent();
    _stackCards.pop_back();
    _relayout();

    return worldPos;
}

void StackView::pushBackCard()
{
    Sprite* card = Sprite::create(CardResConfig::getCoveredPath());
    if (card)
    {
        card->setAnchorPoint(Vec2(0.5f, 0.5f));
        addChild(card, static_cast<int>(_stackCards.size()));
        _stackCards.push_back(card);
        _relayout();
    }
}

int StackView::getRemainingCount() const
{
    return static_cast<int>(_stackCards.size());
}

void StackView::setOnClickCallback(std::function<void()> callback)
{
    _onClickCallback = callback;
}

void StackView::_setupTouchListener()
{
    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    auto makeBounds = [this]() -> Rect
    {
        int n = static_cast<int>(_stackCards.size());
        float spreadWidth = (n > 1) ? (n - 1) * kStackCardOffsetX : 0.0f;
        float totalWidth  = CardView::kCardWidth + spreadWidth;
        return Rect(-totalWidth * 0.5f,
                    -CardView::kCardHeight * 0.5f,
                    totalWidth,
                    CardView::kCardHeight);
    };

    listener->onTouchBegan = [this, makeBounds](Touch* touch, Event*) -> bool
    {
        if (_stackCards.empty()) return false;
        Vec2 localPos = convertToNodeSpace(touch->getLocation());
        return makeBounds().containsPoint(localPos);
    };

    listener->onTouchEnded = [this, makeBounds](Touch* touch, Event*)
    {
        Vec2 localPos = convertToNodeSpace(touch->getLocation());
        if (makeBounds().containsPoint(localPos) && _onClickCallback)
            _onClickCallback();
    };

    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
}
