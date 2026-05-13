#ifndef BGMPLAYER_H
#define BGMPLAYER_H

#include <QObject>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QRandomGenerator>
#include <QDebug>

class BGMPlayer : public QObject
{
    Q_OBJECT
public:
    static BGMPlayer& instance()
    {
        static BGMPlayer instance;
        return instance;
    }

    void playBGM(const QString& filePath, bool loop = true)
    {
        QUrl sourceUrl;
        if (filePath.startsWith(':')) {
            // Qt资源文件路径，需使用qrc scheme
            sourceUrl = QUrl("qrc" + filePath);
        } else {
            sourceUrl = QUrl::fromLocalFile(filePath);
        }

        player.setSource(sourceUrl);
        player.setAudioOutput(&audioOutput);
        audioOutput.setVolume(0.3);

        if (loop) {
            player.setLoops(QMediaPlayer::Infinite);
        } else {
            player.setLoops(1);
        }

        player.play();
    }

    void playRandomBGM(const QStringList& paths, bool loop = true)
    {
        if (paths.isEmpty()) return;
        int idx = QRandomGenerator::global()->bounded(paths.size());
        playBGM(paths[idx], loop);
    }
  //  ---------------------暂停和重新播放-------------------
//后面就没再用暂停功能了
    void pause()
    {
        player.pause();
    }

    void resume()
    {
        player.play();
    }

    void stop()
    {
        player.stop();
        player.setPosition(0);
    }
//
    void setVolume(qreal volume)
    {
        audioOutput.setVolume(volume);
    }
//
private:
    explicit BGMPlayer(QObject *parent = nullptr)
        : QObject(parent)
    {
        connect(&player, &QMediaPlayer::errorOccurred, this, [](QMediaPlayer::Error error, const QString &errorString) {
            qDebug() << "BGM播放错误:" << error << errorString;
        });
    }

    QMediaPlayer player;
    QAudioOutput audioOutput;

    BGMPlayer(const BGMPlayer&) = delete;
    BGMPlayer& operator=(const BGMPlayer&) = delete;
};

#endif // BGMPLAYER_H