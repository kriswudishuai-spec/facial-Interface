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
    // 简单模拟：在当前分数附近小幅波动，并偶尔大幅下降
    double delta = (QRandomGenerator::global()->bounded(2001) - 1000) / 100000.0; // [-0.01, 0.01]
    currentScore_ += delta;

    if (QRandomGenerator::global()->bounded(100) < 5) {
        // 5% 概率模拟一次“走神”
        currentScore_ = 0.3 + QRandomGenerator::global()->bounded(2001) / 10000.0; // [0.3, 0.5]
    }

    if (currentScore_ < 0.0) currentScore_ = 0.0;
    if (currentScore_ > 1.0) currentScore_ = 1.0;

    emit attentionUpdated(currentScore_);
}

