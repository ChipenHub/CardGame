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

    for (size_t i = 0; i < cards.size(); ++i)
    {
        for (size_t j = i + 1; j < cards.size(); ++j)   // j > i  ⇒  j 是后放的牌，层级更高
        {
            const auto& A = cards[i];   // 先放的牌（可能被遮挡）
            const auto& B = cards[j];   // 后放的牌（可能遮挡别人）

            // 计算两张牌的矩形（中心点）
            cocos2d::Rect rectA(A.position.x - halfW, A.position.y - halfH, cardWidth, cardHeight);
            cocos2d::Rect rectB(B.position.x - halfW, B.position.y - halfH, cardWidth, cardHeight);

            if (rectA.intersectsRect(rectB))
            {
                // 后放的 B 遮挡先放的 A
                // 所以 A 被 B 遮挡
                cards[i].coveredByIds.push_back(B.id);
            }
        }
    }
}