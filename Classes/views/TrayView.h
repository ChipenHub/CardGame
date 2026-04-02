#ifndef __TRAY_VIEW_H__
#define __TRAY_VIEW_H__

#include "cocos2d.h"
#include "CardView.h"
#include <vector>
#include <functional>

/**
 * TrayView
 * 手牌区 / 底牌区视图
 *
 * 始终在中心位置展示一张正面朝上的卡牌（顶部牌）
 * 内部维护一个 CardView 栈，用于支持 Undo 时恢复上一张
 */
class TrayView : public cocos2d::Node
{
public:
    static TrayView* create();

    /**
     * 设置顶部牌（无动画，用于初始化）
     * 旧顶部牌被压入内部栈（不移除，仅隐藏）
     */
    void setTopCard(CardView* cardView);

    /** 获取当前顶部牌视图，无牌返回 nullptr */
    CardView* getTopCardView() const;

    /**
     * 获取顶部牌的世界坐标（即 TrayView 中心的世界坐标）
     * 用于计算飞行动画终点
     */
    cocos2d::Vec2 getTopCardWorldPosition() const;

    /**
     * 播放新牌飞入动画：牌从 fromWorldPos 飞到 TrayView 中心，完成后成为新顶部牌
     * @param cardView    新牌视图（调用前已 addChild 到场景或父节点）
     * @param fromWorldPos 飞行起点世界坐标
     * @param duration    飞行时长
     * @param onComplete  完成回调
     */
    void playCardFlyInAnimation(CardView* cardView,
                                 const cocos2d::Vec2& fromWorldPos,
                                 float duration = 0.3f,
                                 std::function<void()> onComplete = nullptr);

    /**
     * Undo：移除当前顶部牌，恢复传入的旧牌为顶部牌
     * @param previousCard 恢复显示的旧顶部牌
     */
    void restorePreviousTopCard(CardView* previousCard);

    /**
     * 移除当前顶部牌视图（用于 Undo 时取走顶牌，由 Controller 再飞回原位）
     * @return 被移除的顶部牌，可能为 nullptr
     */
    CardView* detachTopCard();

private:
    bool init() override;

    CardView*                _topCardView;
    std::vector<CardView*>   _cardStack;   // 历史牌栈（底层），用于 Undo 恢复
};

#endif // __TRAY_VIEW_H__
