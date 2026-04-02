#include "LevelConfigLoader.h"
#include "cocos2d.h"
#include "json/document.h"

USING_NS_CC;

LevelConfig LevelConfigLoader::loadFromFile(const std::string& filePath)
{
    LevelConfig config;

    std::string content = FileUtils::getInstance()->getStringFromFile(filePath);
    if (content.empty())
    {
        CCLOGERROR("LevelConfigLoader: failed to read file: %s", filePath.c_str());
        return config;
    }

    rapidjson::Document doc;
    doc.Parse(content.c_str());
    if (doc.HasParseError())
    {
        CCLOGERROR("LevelConfigLoader: JSON parse error in file: %s", filePath.c_str());
        return config;
    }

    // 解析辅助 lambda
    auto parseCardConfig = [](const rapidjson::Value& obj) -> CardConfig
    {
        CardConfig card;
        if (obj.HasMember("CardFace") && obj["CardFace"].IsInt())
            card.face = static_cast<CardFaceType>(obj["CardFace"].GetInt());
        if (obj.HasMember("CardSuit") && obj["CardSuit"].IsInt())
            card.suit = static_cast<CardSuitType>(obj["CardSuit"].GetInt());
        if (obj.HasMember("Position") && obj["Position"].IsObject())
        {
            const auto& pos = obj["Position"];
            float x = pos.HasMember("x") ? pos["x"].GetFloat() : 0.0f;
            float y = pos.HasMember("y") ? pos["y"].GetFloat() : 0.0f;
            card.position = Vec2(x, y);
        }
        return card;
    };

    // 解析 Playfield
    if (doc.HasMember("Playfield") && doc["Playfield"].IsArray())
    {
        const auto& arr = doc["Playfield"];
        for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
            config.playfieldCards.push_back(parseCardConfig(arr[i]));
    }

    // 解析 Stack
    if (doc.HasMember("Stack") && doc["Stack"].IsArray())
    {
        const auto& arr = doc["Stack"];
        for (rapidjson::SizeType i = 0; i < arr.Size(); ++i)
            config.stackCards.push_back(parseCardConfig(arr[i]));
    }

    return config;
}
