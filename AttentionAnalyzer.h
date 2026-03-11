#ifndef ATTENTIONANALYZER_H
#define ATTENTIONANALYZER_H

#include <QObject>
#include <QString>
#include <QTimer>

class AttentionAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit AttentionAnalyzer(QObject *parent = nullptr);

    void start();
    void stop();

public slots:
    // emotion label from emotion analyzer
    void setEmotion(const QString &label);

signals:
    // 0.0 ~ 1.0 之间的专注度
    void attentionUpdated(double score);

private slots:
    void generateFakeAttention();

private:
    QTimer timer_;
    double currentScore_ = 0.8;
    QString lastEmotion_;  // 上一次收到的表情标签
};

#endif // ATTENTIONANALYZER_H

