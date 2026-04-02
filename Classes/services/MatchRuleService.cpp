#include "MatchRuleService.h"
#include "services/CardAbilityRegistry.h"
#include <cstdlib>

bool MatchRuleService::canMatch(const CardModel& a, const CardModel& b)
{
    if (a.face == CFT_NONE || b.face == CFT_NONE)
        return false;

    // 能力覆盖：若任意一张牌注册了"可与任意牌匹配"能力，直接成立
    ICardAbility* abilityA = CardAbilityRegistry::getInstance().getAbility(a.face, a.suit);
    ICardAbility* abilityB = CardAbilityRegistry::getInstance().getAbility(b.face, b.suit);
    if ((abilityA && abilityA->canMatchWithAny()) ||
        (abilityB && abilityB->canMatchWithAny()))
    {
        return true;
    }

    // 标准规则：点数差 1，K(12) 与 A(0) 循环
    int diff = std::abs(static_cast<int>(a.face) - static_cast<int>(b.face));
    return diff == 1 || diff == (CFT_NUM_CARD_FACE_TYPES - 1);
}
