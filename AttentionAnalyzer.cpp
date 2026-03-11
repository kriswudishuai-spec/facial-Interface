#include "AttentionAnalyzer.h"
#include <QRandomGenerator>

AttentionAnalyzer::AttentionAnalyzer(QObject *parent)
    : QObject(parent)
{
    connect(&timer_, &QTimer::timeout,
            this, &AttentionAnalyzer::generateFakeAttention);
    timer_.setInterval(500); // 每 500ms 更新一次
}

void AttentionAnalyzer::setEmotion(const QString &label)
{
    // 保留最新的表情标签，用于后续生成逻辑
    lastEmotion_ = label;
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
    // 根据最后的表情调整基准分数
    if (!lastEmotion_.isEmpty()) {
        if (lastEmotion_ == "Neutral") {
            // 持续中性 -> 逐渐降低注意力
            currentScore_ += (0.1 - currentScore_) * 0.1;
        } else if (lastEmotion_ == "Surprise") {
            // 持续惊讶 -> 逐渐提升注意力
            currentScore_ += (0.9 - currentScore_) * 0.1;
        } else {
            // 其他表情使用原有随机逻辑
            double delta = (QRandomGenerator::global()->bounded(4001) - 2000) / 10000.0;
            currentScore_ += delta;
        }
    } else {
        // 默认随机波动
        double delta = (QRandomGenerator::global()->bounded(4001) - 2000) / 10000.0;
        currentScore_ += delta;
    }

    if (QRandomGenerator::global()->bounded(100) < 15) {
        currentScore_ = 0.1 + QRandomGenerator::global()->bounded(4001) / 10000.0;
    }

    if (currentScore_ < 0.0) currentScore_ = 0.0;
    if (currentScore_ > 1.0) currentScore_ = 1.0;

    emit attentionUpdated(currentScore_);
}

