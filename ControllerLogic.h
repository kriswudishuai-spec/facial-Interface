#ifndef CONTROLLERLOGIC_H
#define CONTROLLERLOGIC_H

#include <QObject>
#include <QQueue>
#include <QDateTime>

#include "DeviceController.h"

class ControllerLogic : public QObject
{
    Q_OBJECT
public:
    explicit ControllerLogic(DeviceController *deviceCtrl, QObject *parent = nullptr);

    void setAutoMode(bool enabled) { autoMode_ = enabled; }
    bool autoMode() const { return autoMode_; }

public slots:
    void onAttentionUpdated(double score);
    void onEnvUpdated(double temperature, double humidity);

private:
    DeviceController *deviceCtrl_;
    bool autoMode_ = true;
    double focusThreshold_ = 0.65;             // 65% 以上触发自动逻辑
    QQueue<QPair<double, qint64>> history_;     // (score, timestamp-ms)
    int relaxMs_ = 30000;                       // 低于阈值超过 30 秒才动作

    // 防止专注度骤升骤降的平滑器
    double smoothedScore_ = 0.0;
    double maxStep_ = 0.05;                     // 每次更新最大变化（5%）

    double lastTemperature_ = 0.0;
    double lastHumidity_ = 0.0;
};

#endif // CONTROLLERLOGIC_H

