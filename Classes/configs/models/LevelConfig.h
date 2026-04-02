#ifndef __LEVEL_CONFIG_H__
#define __LEVEL_CONFIG_H__

#include "cocos2d.h"
#include "utils/CardEnums.h"
#include <vector>

/**
 * LevelConfig.h
 * 关卡静态配置数据结构
 * 由 LevelConfigLoader 从 JSON 解析生成，运行时只读
 */

/**
 * 单张卡牌的静态配置
 * Playfield 牌使用 position；Stack 牌 position 无实际意义
 */
struct CardConfig
{
    CardFaceType  face;
    CardSuitType  suit;
    cocos2d::Vec2 position;

    CardConfig() : face(CFT_NONE), suit(CST_NONE), position(cocos2d::Vec2::ZERO) {}
};

/**
 * 关卡整体配置
 * playfieldCards：主牌区所有牌，含坐标
 * stackCards    ：[0] 为初始底牌（翻开），[1..] 为备用牌堆（背面朝上）
 */
struct LevelConfig
{
    std::vector<CardConfig> playfieldCards;
    std::vector<CardConfig> stackCards;
};

#endif // __LEVEL_CONFIG_H__
