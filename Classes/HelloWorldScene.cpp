#include "HelloWorldScene.h"

USING_NS_CC;

cocos2d::Scene* HelloWorldScene::createScene()
{
    return HelloWorldScene::create();
}

bool HelloWorldScene::init()
{
    if (!Scene::init())
        return false;

    _gameController = new (std::nothrow) GameController();
    if (_gameController)
        _gameController->startGame("levels/level1.json", this);

    return true;
}
