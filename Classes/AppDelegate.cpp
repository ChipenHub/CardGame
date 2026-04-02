#include "AppDelegate.h"
#include "HelloWorldScene.h"

USING_NS_CC;

AppDelegate::AppDelegate() {}
AppDelegate::~AppDelegate() {}

void AppDelegate::initGLContextAttrs()
{
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};
    GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching()
{
    auto director = Director::getInstance();
    auto glview   = director->getOpenGLView();

    if (!glview)
    {
        glview = GLViewImpl::createWithRect(
            "CardGame",
            cocos2d::Rect(0, 0, 1080, 2080),
            0.5f);
        director->setOpenGLView(glview);
    }

    glview->setDesignResolutionSize(1080, 2080, ResolutionPolicy::FIXED_WIDTH);

    director->setDisplayStats(false);
    director->setAnimationInterval(1.0f / 60.0f);

    auto scene = HelloWorldScene::createScene();
    director->runWithScene(scene);

    return true;
}

void AppDelegate::applicationDidEnterBackground()
{
    Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground()
{
    Director::getInstance()->startAnimation();
}
