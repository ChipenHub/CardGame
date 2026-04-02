#ifndef __I_CARD_ABILITY_H__
#define __I_CARD_ABILITY_H__

#include <vector>

// 前向声明，避免循环依赖
struct GameModel;

/**
 * ICardAbility
 * 功能牌能力接口
 *
 * 每种功能牌实现此接口，注册到 CardAbilityRegistry。
 * 普通牌不注册，能力为 nullptr，走默认逻辑。
 *
 * 新增功能牌时：
 *   1. 实现此接口的子类
 *   2. 在游戏初始化时调用 CardAbilityRegistry::registerAbility() 注册
 *   3. 无需改动任何 Controller / Service 代码
 */
class ICardAbility
{
public:
    virtual ~ICardAbility() = default;

    /**
     * 是否可与任意牌匹配（忽略标准点数规则）
     * 例：Joker 牌可与任意牌匹配
     * 默认：false（走标准匹配规则）
     */
    virtual bool canMatchWithAny() const { return false; }

    /**
     * 当此牌被成功消除后触发
     * 可用于实现链式消除、炸弹等效果
     * @param cardId  被消除的牌 ID
     * @param model   游戏数据模型（可读写）
     * @return 额外需要从主牌区移除的牌 ID 列表（空列表 = 无额外效果）
     *
     * 注意：返回的牌 ID 由 PlayFieldController 负责从视图和模型中移除；
     *       此方法只做数据层决策，不直接操作视图。
     */
    virtual std::vector<int> onRemoved(int cardId, GameModel& model) { return {}; }
};

#endif // __I_CARD_ABILITY_H__
