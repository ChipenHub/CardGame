#ifndef __APP_DELEGATE_H__
#define __APP_DELEGATE_H__

#include "cocos2d.h"

/**
 * AppDelegate
 * 应用程序入口，负责初始化 Director 和启动第一个场景
 */
class AppDelegate : private cocos2d::Application
{
public:
    AppDelegate();
    virtual ~AppDelegate();

    virtual void initGLContextAttrs() override;

    /** 初始化完成，启动游戏场景 */
    virtual bool applicationDidFinishLaunching() override;

    virtual void applicationDidEnterBackground() override;
    virtual void applicationWillEnterForeground() override;
};

#endif // __APP_DELEGATE_H__
