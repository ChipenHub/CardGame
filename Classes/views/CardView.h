#ifndef __CARD_VIEW_H__
#define __CARD_VIEW_H__

#include "cocos2d.h"
#include "utils/CardEnums.h"
#include <functional>

/**
 * CardView
 * 单张卡牌的视图节点
 *
 * 正面由以下精灵拼合：
 *   card_bg（白色底色） + 大数字（中心偏下） + 小数字（左上角） + 花色图标（右上角）
 * 背面：card_covered 覆盖整张牌
 *
 * 卡牌尺寸由 card_bg.png 决定（182×282），不缩放
 * 锚点设为 (0.5, 0.5)
 */
class CardView : public cocos2d::Node
{
public:
    /**
     * 创建卡牌视图
     * @param cardId 对应 CardModel.id
     * @param face   点数
     * @param suit   花色
     */
    static CardView* create(int cardId, CardFaceType face, CardSuitType suit);

    /** @return 卡牌 ID */
    int getCardId() const { return _cardId; }

    // -------------------------------------------------------------------------
    // 显示状态
    // -------------------------------------------------------------------------

    /** 显示正面（card_bg + 数字 + 花色） */
    void showFront();

    /** 显示背面（card_covered），隐藏正面元素 */
    void showBack();

    /** @return 当前是否正面朝上 */
    bool isFront() const { return _isFront; }

    // -------------------------------------------------------------------------
    // 动画（Phase 2 完善，Phase 1 提供空实现保证编译）
    // -------------------------------------------------------------------------

    /**
     * 翻牌动画：背面 → 正面
     * 先沿 X 轴缩放到 0，切换为正面，再展开
     * @param duration   总时长（秒）
     * @param onComplete 动画结束回调
     */
    void playFlipToFrontAnimation(float duration = 0.3f,
                                   std::function<void()> onComplete = nullptr);

    /**
     * 移动到目标位置（世界坐标）
     * @param targetWorldPos 目标世界坐标
     * @param duration       时长（秒）
     * @param onComplete     完成回调
     */
    void playMoveToAnimation(const cocos2d::Vec2& targetWorldPos,
                              float duration = 0.3f,
                              std::function<void()> onComplete = nullptr);

    // -------------------------------------------------------------------------
    // 点击交互
    // -------------------------------------------------------------------------

    /** 设置点击回调，参数为 cardId */
    void setClickCallback(std::function<void(int)> callback);

    /** 启用/禁用点击响应 */
    void setClickEnabled(bool enabled);

    // 卡牌像素尺寸常量
    static const float kCardWidth;
    static const float kCardHeight;

private:
    bool init(int cardId, CardFaceType face, CardSuitType suit);

    int          _cardId;
    CardFaceType _face;
    CardSuitType _suit;
    bool         _isFront;
    bool         _clickEnabled;

    cocos2d::Sprite* _cardBg;       // 正面底色
    cocos2d::Sprite* _bigNumber;    // 大数字（中心偏下）
    cocos2d::Sprite* _smallNumber;  // 小数字（左上角）
    cocos2d::Sprite* _suitIcon;     // 花色图标（右上角）
    cocos2d::Sprite* _coveredSprite;// 牌背

    std::function<void(int)>        _clickCallback;
    cocos2d::EventListenerTouchOneByOne* _touchListener;

    /** 创建并定位正面各精灵 */
    void _setupFrontFace();

    /** 创建牌背精灵 */
    void _setupBackFace();

    /** 注册触摸事件监听器 */
    void _setupTouchListener();
};

#endif // __CARD_VIEW_H__
