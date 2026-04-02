#ifndef __CARD_ENUMS_H__
#define __CARD_ENUMS_H__

/**
 * CardEnums.h
 * 全局枚举定义：花色类型和点数类型
 * 供所有模块共享，无需 .cpp 实现
 */

// 花色类型
enum CardSuitType
{
    CST_NONE = -1,
    CST_CLUBS,      // 梅花 → black
    CST_DIAMONDS,   // 方块 → red
    CST_HEARTS,     // 红桃 → red
    CST_SPADES,     // 黑桃 → black
    CST_NUM_CARD_SUIT_TYPES
};

// 点数类型（值从0开始，用于差值计算）
enum CardFaceType
{
    CFT_NONE = -1,
    CFT_ACE,    // 0
    CFT_TWO,    // 1
    CFT_THREE,  // 2
    CFT_FOUR,   // 3
    CFT_FIVE,   // 4
    CFT_SIX,    // 5
    CFT_SEVEN,  // 6
    CFT_EIGHT,  // 7
    CFT_NINE,   // 8
    CFT_TEN,    // 9
    CFT_JACK,   // 10
    CFT_QUEEN,  // 11
    CFT_KING,   // 12
    CFT_NUM_CARD_FACE_TYPES
};

#endif // __CARD_ENUMS_H__
