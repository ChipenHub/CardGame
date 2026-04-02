#include "CardView.h"
#include "configs/models/CardResConfig.h"

USING_NS_CC;

const float CardView::kCardWidth  = 182.0f;
const float CardView::kCardHeight = 282.0f;

// 正面元素边距（像素）
static const float kPadding = 10.0f;

CardView* CardView::create(int cardId, CardFaceType face, CardSuitType suit)
{
    CardView* view = new (std::nothrow) CardView();
    if (view && view->init(cardId, face, suit))
    {
        view->autorelease();
        return view;
    }
    CC_SAFE_DELETE(view);
    return nullptr;
}

bool CardView::init(int cardId, CardFaceType face, CardSuitType suit)
{
    if (!Node::init())
        return false;

    _cardId       = cardId;
    _face         = face;
    _suit         = suit;
    _isFront      = false;
    _clickEnabled = true;
    _touchListener = nullptr;

    _cardBg       = nullptr;
    _bigNumber    = nullptr;
    _smallNumber  = nullptr;
    _suitIcon     = nullptr;
    _coveredSprite = nullptr;

    setContentSize(Size(kCardWidth, kCardHeight));
    setAnchorPoint(Vec2(0.5f, 0.5f));

    _setupBackFace();
    _setupFrontFace();
    _setupTouchListener();

    // 初始显示背面
    showBack();

    return true;
}

void CardView::_setupFrontFace()
{
    // --- 底色 ---
    _cardBg = Sprite::create(CardResConfig::getCardBgPath());
    if (_cardBg)
    {
        _cardBg->setAnchorPoint(Vec2(0.5f, 0.5f));
        _cardBg->setPosition(Vec2(0.0f, 0.0f));
        addChild(_cardBg, 0);
    }

    // --- 大数字（中心偏下） ---
    _bigNumber = Sprite::create(CardResConfig::getBigNumberPath(_face, _suit));
    if (_bigNumber)
    {
        _bigNumber->setAnchorPoint(Vec2(0.5f, 0.5f));
        _bigNumber->setPosition(Vec2(0.0f, -kCardHeight * 0.08f));
        addChild(_bigNumber, 1);
    }

    // --- 小数字（左上角） ---
    _smallNumber = Sprite::create(CardResConfig::getSmallNumberPath(_face, _suit));
    if (_smallNumber)
    {
        _smallNumber->setAnchorPoint(Vec2(0.0f, 1.0f));
        _smallNumber->setPosition(Vec2(-kCardWidth * 0.5f + kPadding,
                                        kCardHeight * 0.5f - kPadding));
        addChild(_smallNumber, 1);
    }

    // --- 花色图标（右上角） ---
    _suitIcon = Sprite::create(CardResConfig::getSuitIconPath(_suit));
    if (_suitIcon)
    {
        _suitIcon->setAnchorPoint(Vec2(1.0f, 1.0f));
        _suitIcon->setPosition(Vec2(kCardWidth * 0.5f - kPadding,
                                     kCardHeight * 0.5f - kPadding));
        addChild(_suitIcon, 1);
    }
}

void CardView::_setupBackFace()
{
    _coveredSprite = Sprite::create(CardResConfig::getCoveredPath());
    if (_coveredSprite)
    {
        _coveredSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
        _coveredSprite->setPosition(Vec2(0.0f, 0.0f));
        addChild(_coveredSprite, 2);
    }
}

void CardView::_setupTouchListener()
{
    _touchListener = EventListenerTouchOneByOne::create();
    _touchListener->setSwallowTouches(true);

    _touchListener->onTouchBegan = [this](Touch* touch, Event*) -> bool
    {
        if (!_clickEnabled || !isVisible()) return false;

        Vec2 localPos = convertToNodeSpace(touch->getLocation());
        Rect bounds(
            -kCardWidth  * 0.5f,
            -kCardHeight * 0.5f,
            kCardWidth,
            kCardHeight);
        return bounds.containsPoint(localPos);
    };

    _touchListener->onTouchEnded = [this](Touch* touch, Event*)
    {
        if (!_clickEnabled) return;

        Vec2 localPos = convertToNodeSpace(touch->getLocation());
        Rect bounds(
            -kCardWidth  * 0.5f,
            -kCardHeight * 0.5f,
            kCardWidth,
            kCardHeight);

        if (bounds.containsPoint(localPos) && _clickCallback)
            _clickCallback(_cardId);
    };

    getEventDispatcher()->addEventListenerWithSceneGraphPriority(_touchListener, this);
}

void CardView::showFront()
{
    _isFront = true;
    if (_cardBg)      _cardBg->setVisible(true);
    if (_bigNumber)   _bigNumber->setVisible(true);
    if (_smallNumber) _smallNumber->setVisible(true);
    if (_suitIcon)    _suitIcon->setVisible(true);
    if (_coveredSprite) _coveredSprite->setVisible(false);
}

void CardView::showBack()
{
    _isFront = false;
    if (_cardBg)      _cardBg->setVisible(false);
    if (_bigNumber)   _bigNumber->setVisible(false);
    if (_smallNumber) _smallNumber->setVisible(false);
    if (_suitIcon)    _suitIcon->setVisible(false);
    if (_coveredSprite) _coveredSprite->setVisible(true);
}

void CardView::playFlipToFrontAnimation(float duration, std::function<void()> onComplete)
{
    // X 轴缩放到 0 → 切换为正面 → X 轴展开到 1
    float half = duration * 0.5f;

    auto scaleOut = ScaleTo::create(half, 0.0f, 1.0f); // scaleX→0, scaleY不变
    auto flip     = CallFunc::create([this]() { showFront(); });
    auto scaleIn  = ScaleTo::create(half, 1.0f, 1.0f);
    auto notify   = CallFunc::create([onComplete]()
    {
        if (onComplete) onComplete();
    });

    runAction(Sequence::create(scaleOut, flip, scaleIn, notify, nullptr));
}

void CardView::playMoveToAnimation(const Vec2& targetWorldPos, float duration,
                                    std::function<void()> onComplete)
{
    // 将世界坐标转为父节点本地坐标
    Vec2 localTarget = targetWorldPos;
    if (getParent())
        localTarget = getParent()->convertToNodeSpace(targetWorldPos);

    auto move   = MoveTo::create(duration, localTarget);
    auto ease   = EaseOut::create(move, 2.0f);
    auto notify = CallFunc::create([onComplete]()
    {
        if (onComplete) onComplete();
    });
    runAction(Sequence::create(ease, notify, nullptr));
}

void CardView::setClickCallback(std::function<void(int)> callback)
{
    _clickCallback = callback;
}

void CardView::setClickEnabled(bool enabled)
{
    _clickEnabled = enabled;
}
