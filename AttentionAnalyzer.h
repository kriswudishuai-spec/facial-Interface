#ifndef ATTENTIONANALYZER_H
#define ATTENTIONANALYZER_H

#include <QObject>
#include <QTimer>

class AttentionAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit AttentionAnalyzer(QObject *parent = nullptr);

    void start();
    void stop();

signals:
    // 0.0 ~ 1.0 之间的专注度
    void attentionUpdated(double score);

private slots:
    void generateFakeAttention();

private:
    QTimer timer_;
    double currentScore_ = 0.8;
};

#endif // ATTENTIONANALYZER_H

