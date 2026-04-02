#ifndef __GAME_MODEL_H__
#define __GAME_MODEL_H__

#include "CardModel.h"
#include <vector>
#include <unordered_map>

/**
 * GameModel
 * 游戏全局运行时数据，包含三个区域的卡牌状态
 *
 * 三个区域：
 *   playfieldCards  — 主牌区，含位置、遮挡关系
 *   stackCardIds    — 备用牌堆（从底到顶），顶部为 back()
 *   trayCardIds     — 手牌区/底牌区（栈），顶部为 back()
 *
 * allCards 作为所有牌的主索引，通过 id 快速查找和修改
 */
struct GameModel
{
    /** 主牌区牌的 ID 列表（顺序即布局顺序） */
    std::vector<int> playfieldCardIds;

    /** 备用牌堆 ID 列表，back() 为堆顶（下一张可翻的牌） */
    std::vector<int> stackCardIds;

    /** 手牌区/底牌区 ID 列表，back() 为当前顶部牌 */
    std::vector<int> trayCardIds;

    /** 所有牌的数据索引，通过 id 访问 */
    std::unordered_map<int, CardModel> allCards;

    /**
     * 获取手牌区顶部牌 ID
     * @return 顶部牌 ID；无牌时返回 -1
     */
    int getTrayTopCardId() const;

    /**
     * 根据 ID 获取卡牌数据的可修改引用
     * 调用前请确保 id 有效
     */
    CardModel& getCard(int id);

    /**
     * 根据 ID 获取卡牌数据的只读引用
     */
    const CardModel& getCard(int id) const;

    /**
     * 检查 ID 是否在主牌区中
     */
    bool isInPlayfield(int id) const;
};

#endif // __GAME_MODEL_H__
