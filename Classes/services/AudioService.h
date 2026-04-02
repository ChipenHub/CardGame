#ifndef __AUDIO_SERVICE_H__
#define __AUDIO_SERVICE_H__

/**
 * AudioService
 * 音频播放的统一入口（静态方法，无状态）
 * 底层使用 cocos2d SimpleAudioEngine
 */
class AudioService
{
public:
    /** 预加载所有音效（游戏启动时调用一次） */
    static void preload();

    /** 循环播放背景音乐 */
    static void playBgm();

    /** 停止背景音乐 */
    static void stopBgm();

    /** 点击音效（undo按钮 / 备用牌堆点击） */
    static void playClick();

    /** 主牌区消除成功音效 */
    static void playClear();

    /** 通关音效 */
    static void playWin();

    /** 无更多步骤音效 */
    static void playLose();
};

#endif // __AUDIO_SERVICE_H__
