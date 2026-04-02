#include "CardAbilityRegistry.h"

CardAbilityRegistry& CardAbilityRegistry::getInstance()
{
    static CardAbilityRegistry instance;
    return instance;
}

void CardAbilityRegistry::registerAbility(CardFaceType face, CardSuitType suit,
                                           std::unique_ptr<ICardAbility> ability)
{
    _abilities[Key{face, suit}] = std::move(ability);
}

ICardAbility* CardAbilityRegistry::getAbility(CardFaceType face, CardSuitType suit) const
{
    auto it = _abilities.find(Key{face, suit});
    return (it != _abilities.end()) ? it->second.get() : nullptr;
}

void CardAbilityRegistry::clear()
{
    _abilities.clear();
}
