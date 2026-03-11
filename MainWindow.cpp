#include "MainWindow.h"

#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QWidget>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    attentionAnalyzer_ = new AttentionAnalyzer(this);
    envSensor_ = new EnvSensor(this);
    deviceController_ = new DeviceController(this);
    controllerLogic_ = new ControllerLogic(deviceController_, this);
    emotionAnalyzer_ = new EmotionAnalyzer(this);

    setupUi();

    // 信号连接
    connect(attentionAnalyzer_, &AttentionAnalyzer::attentionUpdated,
            this, &MainWindow::onAttentionUpdated);
    connect(attentionAnalyzer_, &AttentionAnalyzer::attentionUpdated,
            controllerLogic_, &ControllerLogic::onAttentionUpdated);

    connect(envSensor_, &EnvSensor::envUpdated,
            this, &MainWindow::onEnvUpdated);
    connect(envSensor_, &EnvSensor::envUpdated,
            controllerLogic_, &ControllerLogic::onEnvUpdated);

    connect(deviceController_, &DeviceController::lightStateChanged,
            this, &MainWindow::onLightStateChanged);
    connect(deviceController_, &DeviceController::curtainStateChanged,
            this, &MainWindow::onCurtainStateChanged);

    connect(emotionAnalyzer_, &EmotionAnalyzer::emotionUpdated,
            this, &MainWindow::onEmotionUpdated, Qt::QueuedConnection);
    // 将表情更新传递给注意力分析器以调整分数策略
    connect(emotionAnalyzer_, &EmotionAnalyzer::emotionUpdated,
            attentionAnalyzer_, &AttentionAnalyzer::setEmotion, Qt::QueuedConnection);
    connect(emotionAnalyzer_, &EmotionAnalyzer::frameUpdated,
            this, &MainWindow::onFrameUpdated, Qt::QueuedConnection);
    connect(emotionAnalyzer_, &EmotionAnalyzer::errorOccurred,
            this, &MainWindow::onEmotionError, Qt::QueuedConnection);

    attentionAnalyzer_->start();
    envSensor_->start();
    emotionAnalyzer_->start(0);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupUi()
{
    auto *central = new QWidget(this);
    central->setStyleSheet(
        "QWidget { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #f8f9fa, stop:1 #e9ecef); "
        "}"
        "QGroupBox { "
        "border: 1px solid #dee2e6; "
        "border-radius: 12px; "
        "margin-top: 12px; "
        "padding-top: 8px; "
        "font-weight: bold; "
        "font-size: 12px; "
        "color: #495057; "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 rgba(255,255,255,0.9), stop:1 rgba(248,249,250,0.8)); "
        "}"
        "QGroupBox::title { "
        "subcontrol-origin: margin; "
        "left: 12px; "
        "padding: 0 8px 0 8px; "
        "color: #007bff; "
        "font-weight: 600; "
        "}"
        "QLabel { "
        "color: #343a40; "
        "font-size: 11px; "
        "}"
        "QProgressBar { "
        "border: 1px solid #ced4da; "
        "border-radius: 8px; "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #f8f9fa, stop:1 #e9ecef); "
        "text-align: center; "
        "}"
        "QProgressBar::chunk { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #28a745, stop:1 #20c997); "
        "border-radius: 7px; "
        "}"
        "QPushButton { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #007bff, stop:1 #0056b3); "
        "color: white; "
        "border: none; "
        "border-radius: 8px; "
        "padding: 10px 16px; "
        "font-weight: 500; "
        "font-size: 11px; "
        "transition: all 0.3s ease; "
        "}"
        "QPushButton:hover { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #0056b3, stop:1 #004085); "
        "transform: translateY(-1px); "
        "box-shadow: 0 4px 8px rgba(0,123,255,0.3); "
        "}"
        "QPushButton:pressed { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #004085, stop:1 #002752); "
        "transform: translateY(0px); "
        "box-shadow: 0 2px 4px rgba(0,123,255,0.2); "
        "}"
    );
    
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Camera/Emotion Recognition Area - with modern bulb indicator
    auto *camGroup = new QGroupBox(tr("Camera & Emotion Recognition"), central);
    auto *camLayout = new QVBoxLayout(camGroup);
    
    // 视频和灯泡的水平布局
    auto *videoAndBulbLayout = new QHBoxLayout;
    videoLabel_ = new QLabel(camGroup);
    videoLabel_->setMinimumSize(480, 360);
    videoLabel_->setAlignment(Qt::AlignCenter);
    videoLabel_->setText(tr("Waiting for camera feed..."));
    videoLabel_->setStyleSheet(
        "QLabel { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #1a1a1a, stop:1 #2d2d2d); "
        "color: #cccccc; "
        "border-radius: 12px; "
        "border: 2px solid #404040; "
        "font-size: 14px; "
        "font-weight: 300; "
        "}"
    );
    videoAndBulbLayout->addWidget(videoLabel_, 1);
    
    // 灯泡指示器
    auto *bulbContainer = new QWidget(camGroup);
    auto *bulbLayout = new QVBoxLayout(bulbContainer);
    bulbLayout->setSpacing(5);
    bulbLayout->setContentsMargins(10, 0, 0, 0);
    
    bulbLabel_ = new QLabel(tr("💡"), bulbContainer);
    bulbLabel_->setAlignment(Qt::AlignCenter);
    bulbLabel_->setStyleSheet(
        "QLabel { "
        "font-size: 64px; "
        "color: #f8d568; "
        "text-shadow: 0px 0px 15px rgba(248, 214, 104, 0.4); "
        "transition: all 0.5s cubic-bezier(0.4, 0, 0.2, 1); "
        "}"
    );
    
    auto *bulbStatusLabel = new QLabel(tr("Emotion Indicator"), bulbContainer);
    bulbStatusLabel->setAlignment(Qt::AlignCenter);
    bulbStatusLabel->setStyleSheet(
        "QLabel { "
        "font-size: 10px; "
        "color: #6c757d; "
        "font-weight: 400; "
        "letter-spacing: 0.5px; "
        "text-transform: uppercase; "
        "}"
    );
    
    bulbLayout->addWidget(bulbLabel_);
    bulbLayout->addWidget(bulbStatusLabel);
    bulbLayout->addStretch();
    
    videoAndBulbLayout->addWidget(bulbContainer, 0);
    camLayout->addLayout(videoAndBulbLayout);
    
    emotionLabel_ = new QLabel(tr("Current Emotion: --"), camGroup);
    emotionLabel_->setStyleSheet(
        "QLabel { "
        "color: #495057; "
        "font-size: 13px; "
        "font-weight: 500; "
        "padding: 8px 0; "
        "background: rgba(0,123,255,0.05); "
        "border-radius: 6px; "
        "border-left: 4px solid #007bff; "
        "}"
    );
    camLayout->addWidget(emotionLabel_);

    // Attention Level Area
    auto *attentionGroup = new QGroupBox(tr("Student Attention Level"), central);
    auto *attentionLayout = new QVBoxLayout(attentionGroup);
    attentionLabel_ = new QLabel(tr("Current Attention: 0%"), attentionGroup);
    attentionLabel_->setStyleSheet(
        "QLabel { "
        "color: #28a745; "
        "font-weight: 600; "
        "font-size: 14px; "
        "text-shadow: 0px 1px 2px rgba(40, 167, 69, 0.2); "
        "}"
    );
    attentionBar_ = new QProgressBar(attentionGroup);
    attentionBar_->setMinimumHeight(25);
    attentionLayout->addWidget(attentionLabel_);
    attentionLayout->addWidget(attentionBar_);

    // 环境信息区域
    auto *envGroup = new QGroupBox(tr("Classroom Environment"), central);
    auto *envLayout = new QVBoxLayout(envGroup);
    tempLabel_ = new QLabel(tr("Temperature: -- °C"), envGroup);
    tempLabel_->setStyleSheet(
        "QLabel { "
        "color: #dc3545; "
        "font-weight: 500; "
        "font-size: 12px; "
        "background: rgba(220, 53, 69, 0.08); "
        "padding: 6px 10px; "
        "border-radius: 6px; "
        "border-left: 3px solid #dc3545; "
        "}"
    );
    humLabel_ = new QLabel(tr("Humidity: -- %"), envGroup);
    humLabel_->setStyleSheet(
        "QLabel { "
        "color: #17a2b8; "
        "font-weight: 500; "
        "font-size: 12px; "
        "background: rgba(23, 162, 184, 0.08); "
        "padding: 6px 10px; "
        "border-radius: 6px; "
        "border-left: 3px solid #17a2b8; "
        "}"
    );
    envLayout->addWidget(tempLabel_);
    envLayout->addWidget(humLabel_);

    // Device Status Area
    auto *deviceGroup = new QGroupBox(tr("Device Status"), central);
    auto *deviceLayout = new QVBoxLayout(deviceGroup);
    lightStatusLabel_ = new QLabel(tr("Light: ON"), deviceGroup);
    lightStatusLabel_->setStyleSheet(
        "QLabel { "
        "color: #ffc107; "
        "font-weight: 500; "
        "font-size: 12px; "
        "background: rgba(255, 193, 7, 0.1); "
        "padding: 6px 10px; "
        "border-radius: 6px; "
        "border-left: 3px solid #ffc107; "
        "}"
    );
    curtainStatusLabel_ = new QLabel(tr("Curtain: CLOSED"), deviceGroup);
    curtainStatusLabel_->setStyleSheet(
        "QLabel { "
        "color: #6f42c1; "
        "font-weight: 500; "
        "font-size: 12px; "
        "background: rgba(111, 66, 193, 0.1); "
        "padding: 6px 10px; "
        "border-radius: 6px; "
        "border-left: 3px solid #6f42c1; "
        "}"
    );
    deviceLayout->addWidget(lightStatusLabel_);
    deviceLayout->addWidget(curtainStatusLabel_);

    // Control Panel Area
    auto *controlGroup = new QGroupBox(tr("Control Panel"), central);
    auto *controlLayout = new QVBoxLayout(controlGroup);

    modeButton_ = new QPushButton(tr("Mode: Auto (Click to Toggle)"), controlGroup);
    modeButton_->setStyleSheet(
        "QPushButton { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #28a745, stop:1 #20c997); "
        "color: white; "
        "border: none; "
        "border-radius: 10px; "
        "padding: 12px 20px; "
        "font-weight: 600; "
        "font-size: 12px; "
        "transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1); "
        "}"
        "QPushButton:hover { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #20c997, stop:1 #17a2b8); "
        "transform: translateY(-2px); "
        "box-shadow: 0 6px 12px rgba(40, 167, 69, 0.4); "
        "}"
        "QPushButton:pressed { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #17a2b8, stop:1 #138496); "
        "transform: translateY(0px); "
        "box-shadow: 0 3px 6px rgba(40, 167, 69, 0.3); "
        "}"
    );
    connect(modeButton_, &QPushButton::clicked,
            this, &MainWindow::onToggleMode);

    auto *lightBtnLayout = new QHBoxLayout;
    lightOnBtn_ = new QPushButton(tr("Turn Light ON"), controlGroup);
    lightOnBtn_->setMinimumHeight(35);
    lightOffBtn_ = new QPushButton(tr("Turn Light OFF"), controlGroup);
    lightOffBtn_->setMinimumHeight(35);
    lightOffBtn_->setStyleSheet(
        "QPushButton { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #6c757d, stop:1 #495057); "
        "color: white; "
        "border: none; "
        "border-radius: 8px; "
        "padding: 10px 16px; "
        "font-weight: 500; "
        "font-size: 11px; "
        "transition: all 0.3s ease; "
        "}"
        "QPushButton:hover { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #495057, stop:1 #343a40); "
        "transform: translateY(-1px); "
        "box-shadow: 0 4px 8px rgba(108, 117, 125, 0.3); "
        "}"
        "QPushButton:pressed { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #343a40, stop:1 #212529); "
        "transform: translateY(0px); "
        "box-shadow: 0 2px 4px rgba(108, 117, 125, 0.2); "
        "}"
    );
    connect(lightOnBtn_, &QPushButton::clicked,
            this, &MainWindow::onManualLightOn);
    connect(lightOffBtn_, &QPushButton::clicked,
            this, &MainWindow::onManualLightOff);
    lightBtnLayout->addWidget(lightOnBtn_);
    lightBtnLayout->addWidget(lightOffBtn_);

    auto *curtainBtnLayout = new QHBoxLayout;
    curtainOpenBtn_ = new QPushButton(tr("Open Curtain"), controlGroup);
    curtainOpenBtn_->setMinimumHeight(35);
    curtainCloseBtn_ = new QPushButton(tr("Close Curtain"), controlGroup);
    curtainCloseBtn_->setMinimumHeight(35);
    curtainCloseBtn_->setStyleSheet(
        "QPushButton { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #6c757d, stop:1 #495057); "
        "color: white; "
        "border: none; "
        "border-radius: 8px; "
        "padding: 10px 16px; "
        "font-weight: 500; "
        "font-size: 11px; "
        "transition: all 0.3s ease; "
        "}"
        "QPushButton:hover { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #495057, stop:1 #343a40); "
        "transform: translateY(-1px); "
        "box-shadow: 0 4px 8px rgba(108, 117, 125, 0.3); "
        "}"
        "QPushButton:pressed { "
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "stop:0 #343a40, stop:1 #212529); "
        "transform: translateY(0px); "
        "box-shadow: 0 2px 4px rgba(108, 117, 125, 0.2); "
        "}"
    );
    connect(curtainOpenBtn_, &QPushButton::clicked,
            this, &MainWindow::onManualCurtainOpen);
    connect(curtainCloseBtn_, &QPushButton::clicked,
            this, &MainWindow::onManualCurtainClose);
    curtainBtnLayout->addWidget(curtainOpenBtn_);
    curtainBtnLayout->addWidget(curtainCloseBtn_);

    controlLayout->addWidget(modeButton_);
    controlLayout->addLayout(lightBtnLayout);
    controlLayout->addLayout(curtainBtnLayout);

    // Layout composition - improved proportions
    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(camGroup, 3);
    topLayout->setSpacing(15);
    
    auto *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(attentionGroup, 1);
    rightLayout->addWidget(envGroup, 1);
    rightLayout->addWidget(deviceGroup, 1);
    rightLayout->setSpacing(15);
    
    topLayout->addLayout(rightLayout, 1);

    mainLayout->addLayout(topLayout, 3);
    mainLayout->addWidget(controlGroup, 1);

    setCentralWidget(central);
    
    // 设置窗口大小和标题
    setWindowTitle(tr("Classroom Emotion & Attention Analysis System"));
    resize(1400, 900);

    updateModeUi();
}

void MainWindow::updateModeUi()
{
    if (autoMode_) {
        modeButton_->setText(tr("Mode: Auto (Click to Toggle)"));
    } else {
        modeButton_->setText(tr("Mode: Manual (Click to Toggle)"));
    }

    controllerLogic_->setAutoMode(autoMode_);
}

void MainWindow::onAttentionUpdated(double score)
{
    int percent = static_cast<int>(score * 100.0);
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    attentionBar_->setValue(percent);
    attentionLabel_->setText(tr("Current Attention: %1%").arg(percent));
}

void MainWindow::onEnvUpdated(double temperature, double humidity)
{
    tempLabel_->setText(tr("Temperature：%1 °C").arg(QString::number(temperature, 'f', 1)));
    humLabel_->setText(tr("Humidity：%1 %").arg(QString::number(humidity, 'f', 1)));
}

void MainWindow::onLightStateChanged(LightState s)
{
    if (s == LightState::On) {
        lightStatusLabel_->setText(tr("Light: ON"));
    } else {
        lightStatusLabel_->setText(tr("Light: OFF"));
    }
}

void MainWindow::onCurtainStateChanged(CurtainState s)
{
    if (s == CurtainState::Open) {
        curtainStatusLabel_->setText(tr("Curtain: OPEN"));
    } else {
        curtainStatusLabel_->setText(tr("Curtain: CLOSED"));
    }
}

void MainWindow::onEmotionUpdated(const QString &label, double confidence)
{
    if (!emotionLabel_)
        return;

    const int confPct = static_cast<int>(confidence * 100.0);
    emotionLabel_->setText(tr("Current Emotion: %1（%2%）").arg(label).arg(confPct));
    
    // 根据表情类型更新灯泡状态 - 更加智能的分类
    // 积极表情：快乐、惊讶 -> 灯泡暗（正常状态）
    // 中性表情：中性 -> 灯泡中等亮度
    // 消极表情：悲伤、愤怒、厌恶、恐惧、蔑视 -> 灯泡亮（警告状态）
    if (bulbLabel_) {
        bool isNegativeEmotion = false;
        bool isNeutralEmotion = false;
        
        // 检查是否为消极表情
        if (label == "Sadness" || label == "Anger" || label == "Disgust" || 
            label == "Fear" || label == "Contempt") {
            isNegativeEmotion = true;
        } else if (label == "Neutral") {
            isNeutralEmotion = true;
        }
        
        if (isNegativeEmotion) {
            // 消极表情 - 灯泡亮（红色；表示警告/需要注意）
            bulbLabel_->setStyleSheet(
                "QLabel { "
                "font-size: 64px; "
                "color: #e74c3c; "          // 亮红色
                "text-shadow: 0px 0px 20px rgba(231, 76, 60, 0.7); "  // 强发光效果
                "transition: all 0.5s cubic-bezier(0.4, 0, 0.2, 1); "
                "}"
            );
        } else if (isNeutralEmotion) {
            // 中性表情 - 灯泡中等亮度（橙黄色；表示一般状态）
            bulbLabel_->setStyleSheet(
                "QLabel { "
                "font-size: 64px; "
                "color: #f39c12; "          // 中等橙黄
                "text-shadow: 0px 0px 12px rgba(243, 156, 18, 0.5); "
                "transition: all 0.5s cubic-bezier(0.4, 0, 0.2, 1); "
                "}"
            );
        } else {
            // 积极表情 - 灯泡暗（淡黄色；表示良好状态）
            bulbLabel_->setStyleSheet(
                "QLabel { "
                "font-size: 64px; "
                "color: #f8d568; "          // 淡黄色
                "text-shadow: 0px 0px 8px rgba(248, 214, 104, 0.3); "
                "transition: all 0.5s cubic-bezier(0.4, 0, 0.2, 1); "
                "}"
            );
        }
    }
}

void MainWindow::onFrameUpdated(const QImage &image)
{
    if (!videoLabel_)
        return;

    const QPixmap pix = QPixmap::fromImage(image);
    videoLabel_->setPixmap(pix.scaled(videoLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWindow::onEmotionError(const QString &message)
{
    if (emotionLabel_)
        emotionLabel_->setText(tr("Emotion Recognition Error: %1").arg(message));
}

void MainWindow::onToggleMode()
{
    autoMode_ = !autoMode_;
    updateModeUi();
}

void MainWindow::onManualLightOn()
{
    if (!autoMode_ && deviceController_) {
        deviceController_->setLight(LightState::On);
    }
}

void MainWindow::onManualLightOff()
{
    if (!autoMode_ && deviceController_) {
        deviceController_->setLight(LightState::Off);
    }
}

void MainWindow::onManualCurtainOpen()
{
    if (!autoMode_ && deviceController_) {
        deviceController_->setCurtain(CurtainState::Open);
    }
}

void MainWindow::onManualCurtainClose()
{
    if (!autoMode_ && deviceController_) {
        deviceController_->setCurtain(CurtainState::Close);
    }
}

