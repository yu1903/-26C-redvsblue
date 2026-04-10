#include "AudioManager.h"

AudioManager::AudioManager(QObject *parent)
    : QObject(parent)
{
    m_audioOutput = new QAudioOutput(this);
    m_player = new QMediaPlayer(this);
    m_player->setAudioOutput(m_audioOutput);

    // 循环播放
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus status){
        if (status == QMediaPlayer::EndOfMedia) {
            m_player->setPosition(0);
            m_player->play();
        }
    });
}

AudioManager::~AudioManager()
{
}

// 单例
AudioManager &AudioManager::instance()
{
    static AudioManager inst;
    return inst;
}

// 播放主界面音乐
void AudioManager::playMenuMusic()
{
    m_player->setSource(QUrl(menuMusic));
    m_player->play();
}

// 游戏默认音乐
void AudioManager::playGameNormalMusic()
{
    m_player->setSource(QUrl(gameNormalMusic));
    m_player->play();
}

// 8000代后音乐
void AudioManager::playGameLateMusic()
{
    m_player->setSource(QUrl(gameLateMusic));
    m_player->play();
}

// 停止
void AudioManager::stopMusic()
{
    m_player->stop();
}
//暂停
void AudioManager::pauseMusic()
{
    m_player->pause();
}
//恢复
void AudioManager::resumeMusic()
{
    m_player->play();
}