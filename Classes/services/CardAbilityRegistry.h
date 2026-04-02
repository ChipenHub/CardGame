#ifndef __CARD_ABILITY_REGISTRY_H__
#define __CARD_ABILITY_REGISTRY_H__

#include "utils/CardEnums.h"
#include "services/ICardAbility.h"
#include <unordered_map>
#include <memory>

/**
 * CardAbilityRegistry
 * 功能牌能力注册表（单例）
 *
 * 将 (CardFaceType, CardSuitType) 映射到 ICardAbility 实例。
 * 普通牌无需注册，getAbility() 返回 nullptr。
 *
 * 用法：
 *   // 注册（游戏启动时，或关卡加载时）
 *   CardAbilityRegistry::getInstance().registerAbility(
 *       CFT_JOKER, CST_NONE, std::make_unique<JokerAbility>());
 *
 *   // 查询（在 MatchRuleService / PlayFieldController 中自动调用）
 *   ICardAbility* ab = CardAbilityRegistry::getInstance().getAbility(face, suit);
 *   if (ab && ab->canMatchWithAny()) { ... }
 */
class CardAbilityRegistry
{
public:
    static CardAbilityRegistry& getInstance();

    /**
     * 注册一种功能牌的能力
     * 相同 (face, suit) 注册两次会覆盖旧能力
     */
    void registerAbility(CardFaceType face, CardSuitType suit,
                         std::unique_ptr<ICardAbility> ability);

    /**
     * 查询能力；若未注册返回 nullptr（即普通牌）
     */
    ICardAbility* getAbility(CardFaceType face, CardSuitType suit) const;

    /** 清除所有注册（测试 / 重新加载关卡时使用） */
    void clear();

private:
    CardAbilityRegistry() = default;

    struct Key {
        CardFaceType face;
        CardSuitType suit;
        bool operator==(const Key& o) const {
            return face == o.face && suit == o.suit;
        }
    };

    struct KeyHash {
        std::size_t operator()(const Key& k) const {
            return std::hash<int>()(k.face) ^ (std::hash<int>()(k.suit) << 16);
        }
    };

    std::unordered_map<Key, std::unique_ptr<ICardAbility>, KeyHash> _abilities;
};

#endif // __CARD_ABILITY_REGISTRY_H__
