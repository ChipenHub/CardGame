#ifndef __CARD_RES_CONFIG_H__
#define __CARD_RES_CONFIG_H__

#include "utils/CardEnums.h"
#include <string>

/**
 * CardResConfig.h / .cpp
 * 卡牌资源路径映射工具类
 * 根据花色和点数返回对应图片的相对路径（相对于 Resources 根目录）
 * 纯静态方法，无状态
 *
 * 路径规则：
 *   大数字：views/number/big_{color}_{face}.png
 *   小数字：views/number/small_{color}_{face}.png
 *   花色  ：views/suits/{suit}.png
 *   牌背  ：views/card_covered.png
 *   牌底色：views/card_bg.png
 */
class CardResConfig
{
public:
    /** 返回花色对应颜色字符串："red" 或 "black" */
    static std::string getColorStr(CardSuitType suit);

    /** 返回点数对应字符串，如 "A"、"2"、"10"、"J"、"Q"、"K" */
    static std::string getFaceStr(CardFaceType face);

    /** 返回大数字图片路径，如 "views/number/big_red_A.png" */
    static std::string getBigNumberPath(CardFaceType face, CardSuitType suit);

    /** 返回小数字图片路径，如 "views/number/small_black_10.png" */
    static std::string getSmallNumberPath(CardFaceType face, CardSuitType suit);

    /** 返回花色图标路径，如 "views/suits/heart.png" */
    static std::string getSuitIconPath(CardSuitType suit);

    /** 返回牌背路径 */
    static std::string getCoveredPath();

    /** 返回牌底色路径 */
    static std::string getCardBgPath();

private:
    CardResConfig() = delete;
};

#endif // __CARD_RES_CONFIG_H__
