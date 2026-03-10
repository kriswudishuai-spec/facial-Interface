#include "ControllerLogic.h"

ControllerLogic::ControllerLogic(DeviceController *deviceCtrl, QObject *parent)
    : QObject(parent)
    , deviceCtrl_(deviceCtrl)
{
}

void ControllerLogic::onAttentionUpdated(double score)
{
    if (!autoMode_ || !deviceCtrl_)
        return;

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    history_.enqueue(qMakePair(score, now));

    // 只保留最近 relaxMs_ 时间窗口的数据
    while (!history_.isEmpty() && now - history_.head().second > relaxMs_) {
        history_.dequeue();
    }

    if (history_.isEmpty())
        return;

    double sum = 0.0;
    for (const auto &p : std::as_const(history_)) {
        sum += p.first;
    }
    double avg = sum / history_.size();

    if (avg >= focusThreshold_) {
        deviceCtrl_->setLight(LightState::On);
        deviceCtrl_->setCurtain(CurtainState::Close);
    } else {
        deviceCtrl_->setLight(LightState::Off);
        deviceCtrl_->setCurtain(CurtainState::Open);
    }
}

void ControllerLogic::onEnvUpdated(double temperature, double humidity)
{
    lastTemperature_ = temperature;
    lastHumidity_ = humidity;

    // 此处预留：将来可根据环境信息修改策略
}

