#include "GameView.h"

USING_NS_CC;

// 布局常量
static const float kScreenW        = 1080.0f;
static const float kScreenH        = 2080.0f;
static const float kBottomAreaH    = 580.0f;
static const float kPlayFieldY     = kBottomAreaH;
static const float kBottomCenterY  = kBottomAreaH;

static const float kStackCenterX   = kScreenW * 0.25f;     // 270
static const float kTrayCenterX    = kScreenW * 0.75f;     // 810
static const float kUndoCenterX    = kScreenW * 0.5f;      // 540
static const float kUndoCenterY    = 80.0f;

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

    _undoEnabled = false;
    setContentSize(Size(kScreenW, kScreenH));
    setAnchorPoint(Vec2::ZERO);

    _initLayout();
    return true;
}

void GameView::_initLayout()
{
    // --- 背景 ---
    _bgSprite = Sprite::create("views/ui/game_bg.png");
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

    // --- 备用牌堆（左半底部） ---
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
        _trayView->setPosition(Vec2(kTrayCenterX, kBottomCenterY * 0.75f + 140));
        addChild(_trayView, 1);
    }

    // --- Undo 按钮 ---
    _setupUndoButton();
}

void GameView::_setupUndoButton()
{
    _undoSprite = Sprite::create("views/ui/undo.png");
    if (!_undoSprite) return;

    _undoSprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    _undoSprite->setPosition(Vec2(kUndoCenterX, kUndoCenterY));
    _undoSprite->setOpacity(80); // 初始禁用状态：半透明
    addChild(_undoSprite, 2);

    auto listener = EventListenerTouchOneByOne::create();
    listener->setSwallowTouches(true);

    // 固定点击热区 285×117，整体上移半个按钮高度对齐图片
    static const float kBtnW = 285.0f;
    static const float kBtnH = 117.0f;
    static const Rect  kBtnRect(0.0f, 0.0f, kBtnW, kBtnH);

    listener->onTouchBegan = [this](Touch* touch, Event*) -> bool
    {
        if (!_undoEnabled || !_undoSprite->isVisible()) return false;
        Vec2 local = _undoSprite->convertToNodeSpace(touch->getLocation());
        return kBtnRect.containsPoint(local);
    };

    listener->onTouchEnded = [this](Touch* touch, Event*)
    {
        if (!_undoEnabled) return;
        Vec2 local = _undoSprite->convertToNodeSpace(touch->getLocation());
        if (!kBtnRect.containsPoint(local)) return;

        // 跳动效果：快速放大 → 回弹 → 恢复
        _undoSprite->stopAllActions();
        auto bounce = Sequence::create(
            ScaleTo::create(0.08f, 1.25f),
            ScaleTo::create(0.06f, 0.88f),
            ScaleTo::create(0.05f, 1.0f),
            CallFunc::create([this]() {
                if (_onUndoClickCallback) _onUndoClickCallback();
            }),
            nullptr);
        _undoSprite->runAction(bounce);
    };

    getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, _undoSprite);
}

void GameView::setOnUndoClickCallback(std::function<void()> callback)
{
    _onUndoClickCallback = callback;
}

void GameView::setUndoButtonEnabled(bool enabled)
{
    _undoEnabled = enabled;
    if (_undoSprite)
        _undoSprite->setOpacity(enabled ? 255 : 80);
}
