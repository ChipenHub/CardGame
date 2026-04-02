#include "GameView.h"

USING_NS_CC;

// 布局常量
static const float kScreenW        = 1080.0f;
static const float kScreenH        = 2080.0f;
static const float kBottomAreaH    = 580.0f;
static const float kPlayFieldY     = kBottomAreaH;          // PlayFieldView 的 y 起点
static const float kBottomCenterY  = kBottomAreaH;          // 底部区域中心 y

static const float kStackCenterX   = kScreenW * 0.25f;     // 270
static const float kTrayCenterX    = kScreenW * 0.75f;     // 810
static const float kUndoCenterX    = kScreenW * 0.5f;      // 540
static const float kUndoCenterY    = 80.0f;

static const float kCardWidth  = 182.0f;
static const float kCardHeight = 282.0f;

GameView* GameView::create()
{
    GameView* view = new (std::nothrow) GameView();
    if (view && view->init())
    {
        view->autorelease();
        return view;
    }
    CC_SAFE_DELETE(view);
    return nullptr;
}

bool GameView::init()
{
    if (!Node::init())
        return false;

    setContentSize(Size(kScreenW, kScreenH));
    setAnchorPoint(Vec2::ZERO);

    _initLayout();
    return true;
}

void GameView::_initLayout()
{
    // --- 背景 ---
    _bgSprite = Sprite::create("views/game_bg.png");
    if (_bgSprite)
    {
        _bgSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
        _bgSprite->setPosition(Vec2(kScreenW * 0.5f, kScreenH * 0.5f));
        addChild(_bgSprite, 0);
    }

    // --- 主牌区 ---
    _playFieldView = PlayFieldView::create();
    if (_playFieldView)
    {
        _playFieldView->setPosition(Vec2(0.0f, kPlayFieldY));
        addChild(_playFieldView, 1);
    }

    // --- 备用牌堆（左半底部，以中心定位） ---
    _stackView = StackView::create();
    if (_stackView)
    {
        _stackView->setPosition(Vec2(kStackCenterX * 1.75f, kBottomCenterY * 0.75f));
        addChild(_stackView, 1);
    }

    // --- 手牌区/底牌区（右半底部） ---
    _trayView = TrayView::create();
    if (_trayView)
    {
        _trayView->setPosition(Vec2(kTrayCenterX, kBottomCenterY * 0.75f + 140 /* Visual fix */));
        addChild(_trayView, 1);
    }

    // --- Undo 按钮 ---
    _setupUndoButton();
}

void GameView::_setupUndoButton()
{
    _undoLabel = Label::createWithSystemFont("Undo", "Arial", 48);
    if (!_undoLabel) return;

    _undoLabel->setPosition(Vec2(kUndoCenterX, kUndoCenterY));
    _undoLabel->setColor(Color3B::WHITE);
    addChild(_undoLabel, 2);

    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    listener->onTouchBegan = [this](Touch* touch, Event*) -> bool
    {
        if (!_undoLabel->isVisible()) return false;
        Vec2 local = _undoLabel->convertToNodeSpace(touch->getLocation());
        Size size  = _undoLabel->getContentSize();
        Rect bounds(-size.width * 0.5f, -size.height * 0.5f, size.width * 1.5f, size.height * 1.5f);
        return bounds.containsPoint(local);
    };

    listener->onTouchEnded = [this](Touch* touch, Event*)
    {
        Vec2 local = _undoLabel->convertToNodeSpace(touch->getLocation());
        Size size  = _undoLabel->getContentSize();
        Rect bounds(-size.width * 0.5f, -size.height * 0.5f, size.width * 1.5f, size.height * 1.5f);
        if (bounds.containsPoint(local) && _onUndoClickCallback)
            _onUndoClickCallback();
    };

    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _undoLabel);
}

void GameView::setOnUndoClickCallback(std::function<void()> callback)
{
    _onUndoClickCallback = callback;
}

void GameView::setUndoButtonEnabled(bool enabled)
{
    if (_undoLabel)
    {
        _undoLabel->setColor(enabled ? Color3B::WHITE : Color3B(120, 120, 120));
        _undoLabel->setOpacity(enabled ? 255 : 150);
    }
}
