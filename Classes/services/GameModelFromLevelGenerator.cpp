#include "GameModelFromLevelGenerator.h"
#include <algorithm>

// card_bg.png 实际像素尺寸（不缩放）
static const float kCardWidth  = 182.0f;
static const float kCardHeight = 282.0f;

GameModel GameModelFromLevelGenerator::generate(const LevelConfig& config)
{
    GameModel model;
    int nextId = 0;

    // --- 1. 构建主牌区 CardModel ---
    std::vector<CardModel> playfieldCards;
    playfieldCards.reserve(config.playfieldCards.size());

    for (const auto& cfg : config.playfieldCards)
    {
        CardModel card;
        card.id       = nextId++;
        card.face     = cfg.face;
        card.suit     = cfg.suit;
        cocos2d::Vec2 t = cfg.position;
        t.x += kCardWidth / 2;
        t.y += kCardHeight / 2;
        card.position = t;
        card.faceUp   = true; // 先设为 false，遮挡计算后再修正
        playfieldCards.push_back(card);
    }

    // --- 2. 计算遮挡关系 ---
    _computeCoverRelations(playfieldCards, kCardWidth, kCardHeight);

    // --- 3. 设置 faceUp：无遮挡则正面朝上 ---
    for (auto& card : playfieldCards)
        card.faceUp = card.coveredByIds.empty();

    // --- 4. 写入 GameModel ---
    for (const auto& card : playfieldCards)
    {
        model.playfieldCardIds.push_back(card.id);
        model.allCards[card.id] = card;
    }

    // --- 5. 构建 Stack 牌 ---
    for (size_t i = 0; i < config.stackCards.size(); ++i)
    {
        const auto& cfg = config.stackCards[i];
        CardModel card;
        card.id       = nextId++;
        card.face     = cfg.face;
        card.suit     = cfg.suit;
        card.position = cocos2d::Vec2::ZERO;
        card.faceUp   = (i == 0); // Stack[0] 为初始底牌，正面朝上

        model.allCards[card.id] = card;

        if (i == 0)
        {
            // Stack[0] → 初始底牌，放入手牌区
            model.trayCardIds.push_back(card.id);
        }
        else
        {
            model.stackCardIds.push_back(card.id);
        }
    }

    // stackCardIds 目前是配置文件顺序 [Stack[1], Stack[2], ..., Stack[N]]
    // StackController 用 back() 弹出，需反转使 back() == Stack[1]（最先弹出）
    std::reverse(model.stackCardIds.begin(), model.stackCardIds.end());

    return model;
}

void GameModelFromLevelGenerator::_computeCoverRelations(
    std::vector<CardModel>& cards,
    float cardWidth,
    float cardHeight)
{
    float halfW = cardWidth  * 0.5f;
    float halfH = cardHeight * 0.5f;

    // 清空之前的遮挡记录（保险起见）
    for (auto& card : cards) {
        card.coveredByIds.clear();
    }

    // 遮挡规则：两牌矩形有重叠，且 B.y > A.y（B 在视觉上方）则 B 遮挡 A
    // 使用 1px 的 y 容差过滤同层牌（完全水平的牌不互相遮挡）
    static const float kYEpsilon = 1.0f;

    for (size_t i = 0; i < cards.size(); ++i)
    {
        for (size_t j = i + 1 
                /* 过去放的牌无需遍历，不可能覆盖当前的牌 */ 
            ; j < cards.size(); ++j)
        {
            if (i == j) continue;

            const auto& A = cards[i];   // 被检查是否被遮挡的牌
            const auto& B = cards[j];   // 候选遮挡牌

            // B 必须在 A 的视觉上方（y 更大）才可能遮挡 A
            // if (B.position.y <= A.position.y + kYEpsilon) continue;

            cocos2d::Rect rectA(A.position.x - halfW, A.position.y - halfH, cardWidth, cardHeight);
            cocos2d::Rect rectB(B.position.x - halfW, B.position.y - halfH, cardWidth, cardHeight);

            if (rectA.intersectsRect(rectB))
            {
                cards[i].coveredByIds.push_back(B.id);
            }
        }
    }
}