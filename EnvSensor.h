#ifndef ENVSENSOR_H
#define ENVSENSOR_H

#include <QObject>
#include <QTimer>

class EnvSensor : public QObject
{
    Q_OBJECT
public:
    explicit EnvSensor(QObject *parent = nullptr);

    void start();
    void stop();

signals:
    void envUpdated(double temperature, double humidity);

private slots:
    void generateFakeEnv();

private:
    QTimer timer_;
    double temperature_ = 24.0;
    double humidity_ = 55.0;
};

#endif // ENVSENSOR_H

