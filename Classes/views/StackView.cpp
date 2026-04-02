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
    // 右端固定在 x = 0（StackView 本地原点），牌向左延伸
    // _stackCards.back() 始终在 x=0，其余依次向左偏移
    int n = static_cast<int>(_stackCards.size());
    for (int i = 0; i < n; ++i)
    {
        // 第 i 张牌距最右端（back）的距离 = (n-1-i) 个 offset
        float x = -static_cast<float>(n - 1 - i) * kStackCardOffsetX;
        _stackCards[i]->setPosition(Vec2(x, 0.0f));
    }

    float totalWidth = CardView::kCardWidth
                       + (n > 1 ? static_cast<float>(n - 1) * kStackCardOffsetX : 0.0f);
    setContentSize(Size(totalWidth, CardView::kCardHeight));
}

cocos2d::Vec2 StackView::popTopCard()
{
    if (_stackCards.empty())
        return Vec2::ZERO;

    Sprite* top = _stackCards.back();
    // 取世界坐标（top->getPosition() 当前在 x=0）
    Vec2 worldPos = convertToWorldSpace(top->getPosition());

    top->removeFromParent();
    _stackCards.pop_back();

    // 不重新排列 — 剩余的牌保持原位，视觉上从右往左减少
    // 仅更新内容尺寸
    int n = static_cast<int>(_stackCards.size());
    float totalWidth = CardView::kCardWidth
                       + (n > 1 ? static_cast<float>(n - 1) * kStackCardOffsetX : 0.0f);
    setContentSize(Size(totalWidth, CardView::kCardHeight));

    return worldPos;
}

void StackView::pushBackCard()
{
    // 新牌加在堆顶（x=0），已有牌不动
    Sprite* card = Sprite::create(CardResConfig::getCoveredPath());
    if (card)
    {
        card->setAnchorPoint(Vec2(0.5f, 0.5f));
        card->setPosition(Vec2(0.0f, 0.0f)); // 右端固定位
        addChild(card, static_cast<int>(_stackCards.size()));
        _stackCards.push_back(card);

        int n = static_cast<int>(_stackCards.size());
        float totalWidth = CardView::kCardWidth
                           + (n > 1 ? static_cast<float>(n - 1) * kStackCardOffsetX : 0.0f);
        setContentSize(Size(totalWidth, CardView::kCardHeight));
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

    // 触摸区域：最右端 x=0，向左延伸 totalWidth
    auto makeBounds = [this]() -> Rect
    {
        int n = static_cast<int>(_stackCards.size());
        float totalWidth = CardView::kCardWidth
                           + (n > 1 ? static_cast<float>(n - 1) * kStackCardOffsetX : 0.0f);
        // 右端：CardWidth/2，左端：-(totalWidth - CardWidth/2)
        return Rect(
            -(totalWidth - CardView::kCardWidth * 0.5f),
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
