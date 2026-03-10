#ifndef EMOTIONANALYZER_H
#define EMOTIONANALYZER_H

#include <QObject>
#include <QImage>
#include <QString>

#include <atomic>
#include <thread>

class EmotionAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit EmotionAnalyzer(QObject *parent = nullptr);
    ~EmotionAnalyzer() override;

    // cameraIndex: 0 通常是默认摄像头
    bool start(int cameraIndex = 0);
    void stop();

signals:
    void emotionUpdated(const QString &label, double confidence); // 置信度 0~1
    void frameUpdated(const QImage &image);
    void errorOccurred(const QString &message);

private:
    void processLoop(int cameraIndex);

    std::atomic<bool> running_{false};
    std::thread worker_;
};

#endif // EMOTIONANALYZER_H

