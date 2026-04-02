#ifndef __HELLO_WORLD_SCENE_H__
#define __HELLO_WORLD_SCENE_H__

#include "cocos2d.h"
#include "controllers/GameController.h"

/**
 * HelloWorldScene
 * 游戏主场景，负责创建并持有 GameController
 */
class HelloWorldScene : public cocos2d::Scene
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init() override;

    CREATE_FUNC(HelloWorldScene);

private:
    GameController* _gameController;
};

#endif // __HELLO_WORLD_SCENE_H__
