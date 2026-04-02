#ifndef __LEVEL_CONFIG_LOADER_H__
#define __LEVEL_CONFIG_LOADER_H__

#include "configs/models/LevelConfig.h"
#include <string>

/**
 * LevelConfigLoader
 * 从 JSON 文件解析关卡配置，生成 LevelConfig
 * 纯静态方法，无状态
 *
 * JSON 格式：
 * {
 *   "Playfield": [{ "CardFace": int, "CardSuit": int, "Position": {"x": float, "y": float} }, ...],
 *   "Stack":     [{ "CardFace": int, "CardSuit": int, "Position": {"x": float, "y": float} }, ...]
 * }
 * Stack[0] 为初始底牌，Stack[1..] 为备用牌堆
 */
class LevelConfigLoader
{
public:
    /**
     * 从文件路径加载关卡配置
     * @param filePath 相对于 Resources 目录的路径，如 "levels/level1.json"
     * @return 解析后的 LevelConfig；若文件不存在或解析失败，返回空 LevelConfig
     */
    static LevelConfig loadFromFile(const std::string& filePath);

private:
    LevelConfigLoader() = delete;
};

#endif // __LEVEL_CONFIG_LOADER_H__
