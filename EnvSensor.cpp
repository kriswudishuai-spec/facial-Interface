#include "EnvSensor.h"
#include <QRandomGenerator>

EnvSensor::EnvSensor(QObject *parent)
    : QObject(parent)
{
    connect(&timer_, &QTimer::timeout,
            this, &EnvSensor::generateFakeEnv);
    timer_.setInterval(2000); // 每 2 秒更新一次
}

void EnvSensor::start()
{
    timer_.start();
}

void EnvSensor::stop()
{
    timer_.stop();
}

void EnvSensor::generateFakeEnv()
{
    // 模拟轻微波动
    double tDelta = (QRandomGenerator::global()->bounded(201) - 100) / 100.0; // [-1.0, 1.0]
    double hDelta = (QRandomGenerator::global()->bounded(201) - 100) / 100.0;

    temperature_ += tDelta * 0.1;
    humidity_ += hDelta * 0.3;

    if (temperature_ < 18.0) temperature_ = 18.0;
    if (temperature_ > 30.0) temperature_ = 30.0;
    if (humidity_ < 30.0) humidity_ = 30.0;
    if (humidity_ > 80.0) humidity_ = 80.0;

    emit envUpdated(temperature_, humidity_);
}

