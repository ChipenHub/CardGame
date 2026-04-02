#ifndef __GAME_MODEL_FROM_LEVEL_GENERATOR_H__
#define __GAME_MODEL_FROM_LEVEL_GENERATOR_H__

#include "configs/models/LevelConfig.h"
#include "models/GameModel.h"

/**
 * GameModelFromLevelGenerator
 * 将静态 LevelConfig 转换为运行时 GameModel
 *
 * 职责：
 *   1. 为所有牌分配唯一 ID
 *   2. 根据卡牌矩形位置自动计算遮挡关系（coveredByIds）
 *   3. 根据遮挡关系设置初始 faceUp 状态
 *   4. 将 Stack[0] 设为初始底牌（放入 trayCardIds），Stack[1..] 为备用牌堆
 *   5. 构建 allCards 索引
 *
 * 无状态，纯静态方法
 */
class GameModelFromLevelGenerator
{
public:
    /**
     * 从关卡配置生成游戏数据模型
     * @param config 由 LevelConfigLoader 解析的关卡配置
     * @return 完整初始化的 GameModel
     */
    static GameModel generate(const LevelConfig& config);

private:
    GameModelFromLevelGenerator() = delete;

    /**
     * 计算主牌区所有牌的遮挡关系
     * 规则：若牌 A 的矩形与牌 B 的矩形有交集，且 A.position.y > B.position.y，
     *       则 B 被 A 遮挡（B.coveredByIds 包含 A.id）
     *
     * @param cards     主牌区 CardModel 列表（会被修改）
     * @param cardWidth  卡牌宽度（像素）
     * @param cardHeight 卡牌高度（像素）
     */
    static void _computeCoverRelations(
        std::vector<CardModel>& cards,
        float cardWidth,
        float cardHeight);
};

#endif // __GAME_MODEL_FROM_LEVEL_GENERATOR_H__
