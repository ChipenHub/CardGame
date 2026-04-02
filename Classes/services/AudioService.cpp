#include "AudioService.h"
#include "SimpleAudioEngine.h"

using namespace CocosDenshion;

static const char* kBgm   = "audio/bgm.mp3";
static const char* kClick = "audio/click.mp3";
static const char* kClear = "audio/clear.mp3";
static const char* kWin   = "audio/win.mp3";
static const char* kLose  = "audio/lose.mp3";

void AudioService::preload()
{
    auto* engine = SimpleAudioEngine::getInstance();
    engine->preloadBackgroundMusic(kBgm);
    engine->preloadEffect(kClick);
    engine->preloadEffect(kClear);
    engine->preloadEffect(kWin);
    engine->preloadEffect(kLose);
}

void AudioService::playBgm()
{
    SimpleAudioEngine::getInstance()->playBackgroundMusic(kBgm, true);
}

void AudioService::stopBgm()
{
    SimpleAudioEngine::getInstance()->stopBackgroundMusic();
}

void AudioService::playClick()
{
    SimpleAudioEngine::getInstance()->playEffect(kClick);
}

void AudioService::playClear()
{
    SimpleAudioEngine::getInstance()->playEffect(kClear);
}

void AudioService::playWin()
{
    SimpleAudioEngine::getInstance()->playEffect(kWin);
}

void AudioService::playLose()
{
    SimpleAudioEngine::getInstance()->playEffect(kLose);
}
