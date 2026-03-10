#ifndef DEVICECONTROLLER_H
#define DEVICECONTROLLER_H

#include <QObject>

enum class LightState { On, Off };
enum class CurtainState { Open, Close };

class DeviceController : public QObject
{
    Q_OBJECT
public:
    explicit DeviceController(QObject *parent = nullptr);

    void setLight(LightState state);
    void setCurtain(CurtainState state);

    LightState lightState() const { return lightState_; }
    CurtainState curtainState() const { return curtainState_; }

signals:
    void lightStateChanged(LightState state);
    void curtainStateChanged(CurtainState state);

private:
    LightState lightState_ = LightState::On;
    CurtainState curtainState_ = CurtainState::Close;
};

#endif // DEVICECONTROLLER_H

