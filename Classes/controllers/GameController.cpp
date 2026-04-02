#include "GameController.h"
#include "configs/loaders/LevelConfigLoader.h"
#include "services/GameModelFromLevelGenerator.h"
#include "views/CardView.h"

USING_NS_CC;

GameController::GameController()
    : _gameView(nullptr)
{}

GameController::~GameController()
{
    // _gameView 由 cocos2d 内存管理，无需手动释放
}

void GameController::startGame(const std::string& levelConfigPath, Node* parentNode)
{
    _initModel(levelConfigPath);
    _initView(parentNode);
    _populateViews();
}

void GameController::_initModel(const std::string& levelConfigPath)
{
    LevelConfig config = LevelConfigLoader::loadFromFile(levelConfigPath);
    _gameModel = GameModelFromLevelGenerator::generate(config);
}

void GameController::_initView(Node* parentNode)
{
    _gameView = GameView::create();
    if (_gameView && parentNode)
    {
        _gameView->setPosition(Vec2::ZERO);
        parentNode->addChild(_gameView, 0);
    }
}

void GameController::_populateViews()
{
    if (!_gameView) return;

    PlayFieldView* pfView    = _gameView->getPlayFieldView();
    StackView*     stackView = _gameView->getStackView();
    TrayView*      trayView  = _gameView->getTrayView();

    // --- 主牌区：为每张牌创建 CardView ---
    for (int cardId : _gameModel.playfieldCardIds)
    {
        const CardModel& card = _gameModel.getCard(cardId);
        CardView* cv = CardView::create(card.id, card.face, card.suit);
        if (!cv) continue;

        if (card.faceUp)
            cv->showFront();
        else
            cv->showBack();

        // // JSON 使用 y-down 坐标系，cocos2d 使用 y-up，需镜像 y
        // static const float kPlayFieldHeight = 1500.0f;
        // float displayY = kPlayFieldHeight - card.position.y;
        // z 值：configY 越大（越靠下/前景）z 越高，确保前景牌渲染在背景牌之上
        int zOrder = static_cast<int>(card.position.y);
        pfView->addCardView(cv, cocos2d::Vec2(card.position.x, card.position.y), -zOrder);
    }

    // --- 备用牌堆：初始化背面牌数量 ---
    stackView->initStack(static_cast<int>(_gameModel.stackCardIds.size()));

    // --- 手牌区：初始底牌（trayCardIds[0]） ---
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
