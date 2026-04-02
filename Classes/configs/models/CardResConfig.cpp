#include "CardResConfig.h"

std::string CardResConfig::getColorStr(CardSuitType suit)
{
    if (suit == CST_DIAMONDS || suit == CST_HEARTS)
        return "red";
    return "black";
}

std::string CardResConfig::getFaceStr(CardFaceType face)
{
    switch (face)
    {
        case CFT_ACE:   return "A";
        case CFT_TWO:   return "2";
        case CFT_THREE: return "3";
        case CFT_FOUR:  return "4";
        case CFT_FIVE:  return "5";
        case CFT_SIX:   return "6";
        case CFT_SEVEN: return "7";
        case CFT_EIGHT: return "8";
        case CFT_NINE:  return "9";
        case CFT_TEN:   return "10";
        case CFT_JACK:  return "J";
        case CFT_QUEEN: return "Q";
        case CFT_KING:  return "K";
        default:        return "";
    }
}

std::string CardResConfig::getBigNumberPath(CardFaceType face, CardSuitType suit)
{
    return "views/number/big_" + getColorStr(suit) + "_" + getFaceStr(face) + ".png";
}

std::string CardResConfig::getSmallNumberPath(CardFaceType face, CardSuitType suit)
{
    return "views/number/small_" + getColorStr(suit) + "_" + getFaceStr(face) + ".png";
}

std::string CardResConfig::getSuitIconPath(CardSuitType suit)
{
    switch (suit)
    {
        case CST_CLUBS:    return "views/suits/club.png";
        case CST_DIAMONDS: return "views/suits/diamond.png";
        case CST_HEARTS:   return "views/suits/heart.png";
        case CST_SPADES:   return "views/suits/spade.png";
        default:           return "";
    }
}

std::string CardResConfig::getCoveredPath()
{
    return "views/ui/card_covered.png";
}

std::string CardResConfig::getCardBgPath()
{
    return "views/ui/card_bg.png";
}
