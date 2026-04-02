#include "GameController.h"
#include "configs/loaders/LevelConfigLoader.h"
#include "services/GameModelFromLevelGenerator.h"
#include "views/CardView.h"

USING_NS_CC;

GameController::GameController()
    : _gameView(nullptr)
    , _statusSprite(nullptr)
    , _isAnimating(false)
{}

GameController::~GameController()
{
    // _gameView 由 cocos2d 引用计数管理，无需手动释放
}

void GameController::startGame(const std::string& levelConfigPath, Node* parentNode)
{
    _initModel(levelConfigPath);
    _initViews(parentNode);
    _initControllers();
    _bindCallbacks();
}

// ============================================================
// 初始化
// ============================================================

void GameController::_initModel(const std::string& levelConfigPath)
{
    LevelConfig config = LevelConfigLoader::loadFromFile(levelConfigPath);
    _gameModel = GameModelFromLevelGenerator::generate(config);
    _undoManager.clear();
}

void GameController::_initViews(Node* parentNode)
{
    _gameView = GameView::create();
    if (_gameView && parentNode)
    {
        _gameView->setPosition(Vec2::ZERO);
        parentNode->addChild(_gameView, 0);
    }
}

void GameController::_initControllers()
{
    if (!_gameView) return;

    PlayFieldView* pfView    = _gameView->getPlayFieldView();
    StackView*     stackView = _gameView->getStackView();
    TrayView*      trayView  = _gameView->getTrayView();

    // PlayFieldController
    _playFieldController.init(&_gameModel, &_undoManager);
    _playFieldController.initView(pfView, trayView);

    // StackController
    _stackController.init(&_gameModel, &_undoManager);
    _stackController.initView(stackView, trayView);

    // 初始化备用牌堆视图数量
    stackView->initStack(static_cast<int>(_gameModel.stackCardIds.size()));

    // 初始手牌区底牌（trayCardIds[0]）
    int trayTopId = _gameModel.getTrayTopCardId();
    if (trayTopId != -1)
    {
        const CardModel& trayCard = _gameModel.getCard(trayTopId);
        CardView* cv = CardView::create(trayCard.id, trayCard.face, trayCard.suit);
        if (cv)
        {
            cv->showFront();
            trayView->setTopCard(cv);
        }
    }
}

void GameController::_bindCallbacks()
{
    if (!_gameView) return;

    PlayFieldView* pfView    = _gameView->getPlayFieldView();
    StackView*     stackView = _gameView->getStackView();

    // 主牌区点击
    pfView->setOnCardClickCallback([this](int cardId)
    {
        if (_isAnimating) return;
        bool matched = _playFieldController.handleCardClick(cardId);
        if (matched)
        {
            // 胜负检测在动画解锁回调中触发（_setAnimationLock(false) → _checkGameState）
        }
    });

    // 备用牌堆点击
    stackView->setOnClickCallback([this]()
    {
        if (_isAnimating) return;
        _stackController.handleStackClick();
    });

    // Undo 按钮
    _gameView->setOnUndoClickCallback([this]()
    {
        if (_isAnimating) return;
        handleUndo();
    });

    // 动画锁回调（子控制器回调此处）
    auto lockCb = [this](bool locked) { _setAnimationLock(locked); };
    _playFieldController.setAnimationLockCallback(lockCb);
    _stackController.setAnimationLockCallback(lockCb);
}

// ============================================================
// Undo
// ============================================================

void GameController::handleUndo()
{
    if (!_undoManager.canUndo()) return;

    UndoRecord record = _undoManager.undo();
    _setAnimationLock(true);

    switch (record.type)
    {
    case UndoRecord::Type::MATCH:
        _playFieldController.undoMatch(record);
        break;
    case UndoRecord::Type::DRAW:
        _stackController.undoDraw(record);
        break;
    }

    // Undo 动画较简单，直接解锁（undoMatch 内部有飞行动画，
    // 若需精准时序可改为在动画回调中解锁）
    _setAnimationLock(false);
}

// ============================================================
// 动画锁 & 胜负检测
// ============================================================

void GameController::_setAnimationLock(bool locked)
{
    _isAnimating = locked;

    // 同步 Undo 按钮可用状态
    if (_gameView)
        _gameView->setUndoButtonEnabled(!locked && _undoManager.canUndo());

    if (!locked)
        _checkGameState();
}

void GameController::_checkGameState()
{
    // 每次重新检测前清除上一次的提示图片（Undo 后状态改变时自然消失）
    if (_statusSprite)
    {
        _statusSprite->removeFromParent();
        _statusSprite = nullptr;
    }

    if (_playFieldController.isPlayFieldCleared())
    {
        _onGameWin();
        return;
    }

    bool hasStack  = _stackController.hasCardsRemaining();
    bool hasMatch  = _playFieldController.hasMatchableCard();

    if (!hasStack && !hasMatch && !_gameModel.playfieldCardIds.empty())
        _onGameLose();
}

// 弹出动画：从 0 缩放 → 略微过冲 → 稳定在 1.0
static void playPopIn(cocos2d::Node* node)
{
    node->setScale(0.0f);
    auto anim = cocos2d::Sequence::create(
        cocos2d::ScaleTo::create(0.18f, 1.12f),
        cocos2d::ScaleTo::create(0.08f, 0.92f),
        cocos2d::ScaleTo::create(0.06f, 1.0f),
        nullptr);
    node->runAction(anim);
}

void GameController::_onGameWin()
{
    if (!_gameView) return;
    auto* sprite = Sprite::create("views/ui/success.png");
    if (!sprite) return;
    sprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    sprite->setPosition(Vec2(540, 1040));
    _gameView->addChild(sprite, 999);
    _statusSprite = sprite;
    playPopIn(sprite);
}

void GameController::_onGameLose()
{
    if (!_gameView) return;
    auto* sprite = Sprite::create("views/ui/no_more_moves.png");
    if (!sprite) return;
    sprite->setAnchorPoint(Vec2(0.5f, 0.5f));
    sprite->setPosition(Vec2(540, 1040));
    _gameView->addChild(sprite, 999);
    _statusSprite = sprite;
    playPopIn(sprite);
}
