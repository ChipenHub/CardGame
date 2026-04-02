#ifndef __STACK_VIEW_H__
#define __STACK_VIEW_H__

#include "cocos2d.h"
#include <vector>
#include <functional>

/**
 * StackView
 * 备用牌堆视图
 *
 * 采用扇形横向展开布局：每张牌向右偏移 kStackCardOffsetX，全部显示牌背
 * 最右侧为堆顶（下一张被翻出的牌）
 * 用户点击任意位置触发回调，Controller 决定是否可翻牌
 */
class StackView : public cocos2d::Node
{
public:
    static StackView* create();

    /**
     * 初始化备用牌堆，显示 cardCount 张背面牌
     * @param cardCount 备用牌数量（不含初始底牌）
     */
    void initStack(int cardCount);

    /**
     * 移除堆顶牌精灵，并返回其世界坐标（作为飞行动画起点）
     * @return 堆顶牌的世界坐标；无牌时返回 Vec2::ZERO
     */
    cocos2d::Vec2 popTopCard();

    /**
     * 恢复一张背面牌到堆顶（Undo 用）
     */
    void pushBackCard();

    /** 剩余备用牌数量 */
    int getRemainingCount() const;

    /** 设置点击回调（用户点击牌堆时触发） */
    void setOnClickCallback(std::function<void()> callback);

    // 每张牌的水平偏移量（像素）
    static const float kStackCardOffsetX;

private:
    bool init() override;
    void _setupTouchListener();
    void _relayout();

    std::vector<cocos2d::Sprite*>   _stackCards;
    std::function<void()>           _onClickCallback;
};

#endif // __STACK_VIEW_H__
