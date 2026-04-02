#ifndef __MATCH_RULE_SERVICE_H__
#define __MATCH_RULE_SERVICE_H__

#include "models/CardModel.h"

/**
 * MatchRuleService
 * 匹配规则判断（纯静态无状态）
 *
 * 标准规则：点数差值为 1，K(12) 与 A(0) 可循环匹配。
 * 扩展规则：若任意一张牌在 CardAbilityRegistry 中注册了
 *           canMatchWithAny() == true 的能力，则直接视为可匹配。
 *
 * 接受 CardModel 引用而非 CardFaceType，保留未来通过卡牌完整数据
 * 判断匹配的扩展空间（如特殊花色、附加属性等）。
 */
class MatchRuleService
{
public:
    /**
     * 判断两张牌是否可匹配
     * @param a 主牌区被点击的牌
     * @param b 手牌区顶部牌
     */
    static bool canMatch(const CardModel& a, const CardModel& b);
};

#endif // __MATCH_RULE_SERVICE_H__
