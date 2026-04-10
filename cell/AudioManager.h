#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QMediaPlayer>
#include <QAudioOutput>
#include <QObject>

// 音频管理器：单例模式，全局控制背景音乐
class AudioManager : public QObject
{
    Q_OBJECT
public:
    static AudioManager& instance();

    // 播放主界面音乐
    void playMenuMusic();
    // 播放游戏默认音乐
    void playGameNormalMusic();
    // 播放游戏后期音乐（8000代后）
    void playGameLateMusic();
    // 停止音乐
    void stopMusic();

    void pauseMusic();    // 暂停音乐
    void resumeMusic();   // 恢复音乐

private:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager() override;

    QMediaPlayer* m_player;
    QAudioOutput* m_audioOutput;

    // 音乐路径（你可以自己替换）
    const QString menuMusic = "qrc:/audio/audio/menu.mp3";
    const QString gameNormalMusic = "qrc:/audio/audio/game_normal.mp3";
    const QString gameLateMusic = "qrc:/audio/audio/game_late.mp3";
};

#endif // AUDIOMANAGER_H