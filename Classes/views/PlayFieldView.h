#ifndef __PLAY_FIELD_VIEW_H__
#define __PLAY_FIELD_VIEW_H__

#include "cocos2d.h"
#include "CardView.h"
#include <unordered_map>
#include <functional>

/**
 * PlayFieldView
 * 主牌区视图容器
 * 管理所有主牌区 CardView 的创建、布局和点击事件转发
 * 视图层不含业务逻辑，点击事件通过回调上报给 Controller
 */
class PlayFieldView : public cocos2d::Node
{
public:
    static PlayFieldView* create();

    /**
     * 添加卡牌视图到主牌区
     * @param cardView 卡牌视图（已配置好 face/suit/faceUp）
     * @param position 在主牌区内的局部坐标（已做 y-up 转换）
     * @param zOrder   渲染层级（configY 越大的牌 z 越高，保证前景牌在上）
     */
    void addCardView(CardView* cardView, const cocos2d::Vec2& position, int zOrder = 1);

    /**
     * 从主牌区移除卡牌视图（不播放动画，动画由 Controller 在移除前播放）
     * @param cardId 要移除的卡牌 ID
     */
    void removeCardView(int cardId);

    /**
     * 获取指定 ID 的卡牌视图
     * @return 若不存在返回 nullptr
     */
    CardView* getCardView(int cardId) const;

    /**
     * 设置卡牌点击回调
     * 当任意卡牌被点击时，回调传入 cardId
     */
    void setOnCardClickCallback(std::function<void(int cardId)> callback);

private:
    bool init() override;

    std::unordered_map<int, CardView*>  _cardViews;
    std::function<void(int)>            _onCardClickCallback;
};

#endif // __PLAY_FIELD_VIEW_H__
