#ifndef __GAME_VIEW_H__
#define __GAME_VIEW_H__

#include "cocos2d.h"
#include "PlayFieldView.h"
#include "StackView.h"
#include "TrayView.h"
#include <functional>

/**
 * GameView
 * 游戏顶层视图容器
 *
 * 布局（1080×2080）：
 *   - 背景：game_bg.png，全屏
 *   - 主牌区 PlayFieldView：y = 580，高 1500
 *   - 底部区域（高 580）：
 *       - StackView（左半）：x = 270, y = 290
 *       - TrayView（右半）：x = 810, y = 290
 *       - Undo 按钮（图片）：x = 540, y = 100
 */
class GameView : public cocos2d::Node
{
public:
    static GameView* create();

    PlayFieldView* getPlayFieldView() const { return _playFieldView; }
    StackView*     getStackView()     const { return _stackView; }
    TrayView*      getTrayView()      const { return _trayView; }

    /** 设置 Undo 按钮点击回调 */
    void setOnUndoClickCallback(std::function<void()> callback);

    /** 启用/禁用 Undo 按钮（禁用时半透明） */
    void setUndoButtonEnabled(bool enabled);

private:
    bool init() override;
    void _initLayout();
    void _setupUndoButton();

    cocos2d::Sprite*    _bgSprite;
    PlayFieldView*      _playFieldView;
    StackView*          _stackView;
    TrayView*           _trayView;
    cocos2d::Sprite*    _undoSprite;      // undo.png 按钮

    bool _undoEnabled;
    std::function<void()> _onUndoClickCallback;
};

#endif // __GAME_VIEW_H__
