#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "AttentionAnalyzer.h"
#include "EnvSensor.h"
#include "DeviceController.h"
#include "ControllerLogic.h"
#include "EmotionAnalyzer.h"

class QLabel;
class QProgressBar;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAttentionUpdated(double score);
    void onEnvUpdated(double temperature, double humidity);
    void onLightStateChanged(LightState s);
    void onCurtainStateChanged(CurtainState s);
    void onEmotionUpdated(const QString &label, double confidence);
    void onFrameUpdated(const QImage &image);
    void onEmotionError(const QString &message);

    void onToggleMode();
    void onManualLightOn();
    void onManualLightOff();
    void onManualCurtainOpen();
    void onManualCurtainClose();

private:
    void setupUi();
    void updateModeUi();

    QLabel *attentionLabel_ = nullptr;
    QProgressBar *attentionBar_ = nullptr;
    QLabel *tempLabel_ = nullptr;
    QLabel *humLabel_ = nullptr;
    QLabel *lightStatusLabel_ = nullptr;
    QLabel *curtainStatusLabel_ = nullptr;
    QLabel *emotionLabel_ = nullptr;
    QLabel *videoLabel_ = nullptr;
    QLabel *bulbLabel_ = nullptr;

    QPushButton *modeButton_ = nullptr;
    QPushButton *lightOnBtn_ = nullptr;
    QPushButton *lightOffBtn_ = nullptr;
    QPushButton *curtainOpenBtn_ = nullptr;
    QPushButton *curtainCloseBtn_ = nullptr;

    AttentionAnalyzer *attentionAnalyzer_ = nullptr;
    EnvSensor *envSensor_ = nullptr;
    DeviceController *deviceController_ = nullptr;
    ControllerLogic *controllerLogic_ = nullptr;
    EmotionAnalyzer *emotionAnalyzer_ = nullptr;

    bool autoMode_ = true;
};

#endif // MAINWINDOW_H

