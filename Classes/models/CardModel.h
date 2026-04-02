#ifndef __CARD_MODEL_H__
#define __CARD_MODEL_H__

#include "cocos2d.h"
#include "utils/CardEnums.h"
#include <vector>

/**
 * CardModel
 * 单张卡牌的运行时数据模型
 * 存储卡牌的所有动态状态，不包含业务逻辑
 * 支持序列化（存档恢复）
 */
struct CardModel
{
    /** 唯一ID，初始化时按顺序分配（0, 1, 2, ...） */
    int id;

    /** 点数 */
    CardFaceType face;

    /** 花色 */
    CardSuitType suit;

    /** 在主牌区的坐标（Stack牌此字段无实际意义） */
    cocos2d::Vec2 position;

    /** 是否正面朝上（可操作）；false 表示被遮挡，显示为牌背 */
    bool faceUp;

    /** 遮挡此牌的其他牌的 ID 列表（仅 Playfield 牌使用）
     *  当此列表变为空时，该牌翻开（faceUp = true）
     */
    std::vector<int> coveredByIds;

    CardModel()
        : id(-1)
        , face(CFT_NONE)
        , suit(CST_NONE)
        , position(cocos2d::Vec2::ZERO)
        , faceUp(false)
    {}
};

#endif // __CARD_MODEL_H__
