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
        "QWidget { background-color: #f5f5f5; }"
        "QGroupBox { border: 2px solid #3498db; border-radius: 8px; margin-top: 8px; padding-top: 8px; font-weight: bold; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px 0 3px; }"
        "QLabel { color: #333333; }"
        "QProgressBar { border: 2px solid #3498db; border-radius: 5px; background-color: #e8f4f8; }"
        "QProgressBar::chunk { background-color: #3498db; border-radius: 3px; }"
        "QPushButton { background-color: #3498db; color: white; border: none; border-radius: 5px; padding: 8px; font-weight: bold; }"
        "QPushButton:hover { background-color: #2980b9; }"
        "QPushButton:pressed { background-color: #1f618d; }"
    );
    
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 摄像头/表情识别区域 - 带灯泡指示器
    auto *camGroup = new QGroupBox(tr("摄像头表情识别"), central);
    auto *camLayout = new QVBoxLayout(camGroup);
    
    // 视频和灯泡的水平布局
    auto *videoAndBulbLayout = new QHBoxLayout;
    videoLabel_ = new QLabel(camGroup);
    videoLabel_->setMinimumSize(480, 360);
    videoLabel_->setAlignment(Qt::AlignCenter);
    videoLabel_->setText(tr("等待摄像头画面..."));
    videoLabel_->setStyleSheet("QLabel { background: #1a1a1a; color: #ddd; border-radius: 8px; }");
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
        "font-size: 60px; "
        "color: #f39c12; "  // 默认暗色橙黄
        "text-shadow: 0px 0px 10px rgba(243, 156, 18, 0.3); "
        "}"
    );
    
    auto *bulbStatusLabel = new QLabel(tr("情绪指示"), bulbContainer);
    bulbStatusLabel->setAlignment(Qt::AlignCenter);
    bulbStatusLabel->setStyleSheet("QLabel { font-size: 12px; color: #666; }");
    
    bulbLayout->addWidget(bulbLabel_);
    bulbLayout->addWidget(bulbStatusLabel);
    bulbLayout->addStretch();
    
    videoAndBulbLayout->addWidget(bulbContainer, 0);
    camLayout->addLayout(videoAndBulbLayout);
    
    emotionLabel_ = new QLabel(tr("当前表情：--"), camGroup);
    emotionLabel_->setStyleSheet("QLabel { color: #2c3e50; font-size: 14px; font-weight: bold; }");
    camLayout->addWidget(emotionLabel_);

    // 专注度区域
    auto *attentionGroup = new QGroupBox(tr("学生课堂专注度"), central);
    auto *attentionLayout = new QVBoxLayout(attentionGroup);
    attentionLabel_ = new QLabel(tr("当前专注度：0%"), attentionGroup);
    attentionLabel_->setStyleSheet("QLabel { color: #27ae60; font-weight: bold; }");
    attentionBar_ = new QProgressBar(attentionGroup);
    attentionBar_->setRange(0, 100);
    attentionBar_->setValue(0);
    attentionLayout->addWidget(attentionLabel_);
    attentionLayout->addWidget(attentionBar_);

    // 环境信息区域
    auto *envGroup = new QGroupBox(tr("教室环境"), central);
    auto *envLayout = new QVBoxLayout(envGroup);
    tempLabel_ = new QLabel(tr("温度：-- °C"), envGroup);
    tempLabel_->setStyleSheet("QLabel { color: #e74c3c; font-weight: bold; }");
    humLabel_ = new QLabel(tr("湿度：-- %"), envGroup);
    humLabel_->setStyleSheet("QLabel { color: #3498db; font-weight: bold; }");
    envLayout->addWidget(tempLabel_);
    envLayout->addWidget(humLabel_);

    // 设备状态区域
    auto *deviceGroup = new QGroupBox(tr("设备状态"), central);
    auto *deviceLayout = new QVBoxLayout(deviceGroup);
    lightStatusLabel_ = new QLabel(tr("灯光：开"), deviceGroup);
    lightStatusLabel_->setStyleSheet("QLabel { color: #f39c12; font-weight: bold; }");
    curtainStatusLabel_ = new QLabel(tr("窗帘：关"), deviceGroup);
    curtainStatusLabel_->setStyleSheet("QLabel { color: #9b59b6; font-weight: bold; }");
    deviceLayout->addWidget(lightStatusLabel_);
    deviceLayout->addWidget(curtainStatusLabel_);

    // 控制按钮区域
    auto *controlGroup = new QGroupBox(tr("控制面板"), central);
    auto *controlLayout = new QVBoxLayout(controlGroup);

    modeButton_ = new QPushButton(tr("当前模式：自动 (点击切换)"), controlGroup);
    modeButton_->setMinimumHeight(40);
    connect(modeButton_, &QPushButton::clicked,
            this, &MainWindow::onToggleMode);

    auto *lightBtnLayout = new QHBoxLayout;
    lightOnBtn_ = new QPushButton(tr("开灯"), controlGroup);
    lightOnBtn_->setMinimumHeight(35);
    lightOffBtn_ = new QPushButton(tr("关灯"), controlGroup);
    lightOffBtn_->setMinimumHeight(35);
    lightOffBtn_->setStyleSheet(
        "QPushButton { background-color: #95a5a6; color: white; border: none; border-radius: 5px; padding: 8px; font-weight: bold; }"
        "QPushButton:hover { background-color: #7f8c8d; }"
        "QPushButton:pressed { background-color: #566573; }"
    );
    connect(lightOnBtn_, &QPushButton::clicked,
            this, &MainWindow::onManualLightOn);
    connect(lightOffBtn_, &QPushButton::clicked,
            this, &MainWindow::onManualLightOff);
    lightBtnLayout->addWidget(lightOnBtn_);
    lightBtnLayout->addWidget(lightOffBtn_);

    auto *curtainBtnLayout = new QHBoxLayout;
    curtainOpenBtn_ = new QPushButton(tr("开窗帘"), controlGroup);
    curtainOpenBtn_->setMinimumHeight(35);
    curtainCloseBtn_ = new QPushButton(tr("关窗帘"), controlGroup);
    curtainCloseBtn_->setMinimumHeight(35);
    curtainCloseBtn_->setStyleSheet(
        "QPushButton { background-color: #95a5a6; color: white; border: none; border-radius: 5px; padding: 8px; font-weight: bold; }"
        "QPushButton:hover { background-color: #7f8c8d; }"
        "QPushButton:pressed { background-color: #566573; }"
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

    // 布局组合 - 改进布局比例
    auto *topLayout = new QHBoxLayout;
    topLayout->addWidget(camGroup, 3);
    topLayout->setSpacing(10);
    
    auto *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(attentionGroup, 1);
    rightLayout->addWidget(envGroup, 1);
    rightLayout->addWidget(deviceGroup, 1);
    rightLayout->setSpacing(10);
    
    topLayout->addLayout(rightLayout, 1);

    mainLayout->addLayout(topLayout, 3);
    mainLayout->addWidget(controlGroup, 1);

    setCentralWidget(central);
    
    // 设置窗口大小和标题
    setWindowTitle(tr("课堂表情与注意力分析系统"));
    resize(1400, 900);

    updateModeUi();
}

void MainWindow::updateModeUi()
{
    if (autoMode_) {
        modeButton_->setText(tr("当前模式：自动 (点击切换)"));
    } else {
        modeButton_->setText(tr("当前模式：手动 (点击切换)"));
    }

    controllerLogic_->setAutoMode(autoMode_);
}

void MainWindow::onAttentionUpdated(double score)
{
    int percent = static_cast<int>(score * 100.0);
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    attentionBar_->setValue(percent);
    attentionLabel_->setText(tr("当前专注度：%1%").arg(percent));
}

void MainWindow::onEnvUpdated(double temperature, double humidity)
{
    tempLabel_->setText(tr("温度：%1 °C").arg(QString::number(temperature, 'f', 1)));
    humLabel_->setText(tr("湿度：%1 %").arg(QString::number(humidity, 'f', 1)));
}

void MainWindow::onLightStateChanged(LightState s)
{
    if (s == LightState::On) {
        lightStatusLabel_->setText(tr("灯光：开"));
    } else {
        lightStatusLabel_->setText(tr("灯光：关"));
    }
}

void MainWindow::onCurtainStateChanged(CurtainState s)
{
    if (s == CurtainState::Open) {
        curtainStatusLabel_->setText(tr("窗帘：开"));
    } else {
        curtainStatusLabel_->setText(tr("窗帘：关"));
    }
}

void MainWindow::onEmotionUpdated(const QString &label, double confidence)
{
    if (!emotionLabel_)
        return;

    const int confPct = static_cast<int>(confidence * 100.0);
    emotionLabel_->setText(tr("当前表情：%1（%2%）").arg(label).arg(confPct));
    
    // 根据表情类型更新灯泡状态
    // 积极表情：快乐、惊讶 -> 灯泡暗（不亮）
    // 消极表情：悲伤、愤怒、厌恶、恐惧、蔑视 -> 灯泡亮（警告）
    if (bulbLabel_) {
        bool isNegativeEmotion = false;
        
        // 检查是否为消极表情
        if (label == "Sadness" || label == "Anger" || label == "Disgust" || 
            label == "Fear" || label == "Contempt") {
            isNegativeEmotion = true;
        }
        
        if (isNegativeEmotion) {
            // 消极表情 - 灯泡亮（红色；表示警告/需要注意）
            bulbLabel_->setStyleSheet(
                "QLabel { "
                "font-size: 60px; "
                "color: #e74c3c; "          // 亮红色
                "text-shadow: 0px 0px 20px rgba(231, 76, 60, 0.7); "  // 发光效果
                "}"
            );
        } else {
            // 其他表情（积极或中性）- 灯泡暗（淡黄色；表示正常）
            bulbLabel_->setStyleSheet(
                "QLabel { "
                "font-size: 60px; "
                "color: #f39c12; "          // 暗黄色
                "text-shadow: 0px 0px 10px rgba(243, 156, 18, 0.3); "
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
        emotionLabel_->setText(tr("表情识别错误：%1").arg(message));
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

