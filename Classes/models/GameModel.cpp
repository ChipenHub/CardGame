#include "GameModel.h"
#include <algorithm>

int GameModel::getTrayTopCardId() const
{
    if (trayCardIds.empty())
        return -1;
    return trayCardIds.back();
}

CardModel& GameModel::getCard(int id)
{
    return allCards.at(id);
}

const CardModel& GameModel::getCard(int id) const
{
    return allCards.at(id);
}

bool GameModel::isInPlayfield(int id) const
{
    return std::find(playfieldCardIds.begin(), playfieldCardIds.end(), id)
           != playfieldCardIds.end();
}
