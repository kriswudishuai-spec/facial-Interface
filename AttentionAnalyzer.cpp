#include "AttentionAnalyzer.h"
#include <QRandomGenerator>

AttentionAnalyzer::AttentionAnalyzer(QObject *parent)
    : QObject(parent)
{
    connect(&timer_, &QTimer::timeout,
            this, &AttentionAnalyzer::generateFakeAttention);
    timer_.setInterval(500); // 每 500ms 更新一次
}

void AttentionAnalyzer::start()
{
    timer_.start();
}

void AttentionAnalyzer::stop()
{
    timer_.stop();
}

void AttentionAnalyzer::generateFakeAttention()
{
    // 增加波动幅度：在当前分数附近大幅波动，并增加走神概率
    double delta = (QRandomGenerator::global()->bounded(4001) - 2000) / 10000.0; // [-0.2, 0.2] - 增加20倍波动
    currentScore_ += delta;

    if (QRandomGenerator::global()->bounded(100) < 15) {
        // 15% 概率模拟一次"走神" - 增加3倍概率
        currentScore_ = 0.1 + QRandomGenerator::global()->bounded(4001) / 10000.0; // [0.1, 0.5] - 扩大范围
    }

    if (currentScore_ < 0.0) currentScore_ = 0.0;
    if (currentScore_ > 1.0) currentScore_ = 1.0;

    emit attentionUpdated(currentScore_);
}

