#include "DeviceController.h"

DeviceController::DeviceController(QObject *parent)
    : QObject(parent)
{
}

void DeviceController::setLight(LightState state)
{
    if (lightState_ == state)
        return;

    lightState_ = state;

    // 这里将来可以替换成真实硬件控制代码
    emit lightStateChanged(lightState_);
}

void DeviceController::setCurtain(CurtainState state)
{
    if (curtainState_ == state)
        return;

    curtainState_ = state;

    // 这里将来可以替换成真实硬件控制代码
    emit curtainStateChanged(curtainState_);
}

