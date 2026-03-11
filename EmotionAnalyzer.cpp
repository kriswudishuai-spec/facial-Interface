#include "EmotionAnalyzer.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>

#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

static const char *kEmotionLabels[8] = {
    "Neutral", "Happiness", "Surprise", "Sadness",
    "Anger", "Disgust", "Fear", "Contempt"
};

static QImage matToQImageBgr(const cv::Mat &bgr)
{
    cv::Mat rgb;
    cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
    QImage img(rgb.data, rgb.cols, rgb.rows, static_cast<int>(rgb.step), QImage::Format_RGB888);
    return img.copy(); // 深拷贝，避免 Mat 生命周期问题
}

EmotionAnalyzer::EmotionAnalyzer(QObject *parent)
    : QObject(parent)
{
}

EmotionAnalyzer::~EmotionAnalyzer()
{
    stop();
}

bool EmotionAnalyzer::start(int cameraIndex)
{
    if (running_)
        return true;

    running_ = true;
    worker_ = std::thread([this, cameraIndex]() { processLoop(cameraIndex); });
    return true;
}

void EmotionAnalyzer::stop()
{
    if (!running_)
        return;

    running_ = false;
    if (worker_.joinable())
        worker_.join();
}

void EmotionAnalyzer::processLoop(int cameraIndex)
{
    // 统一从可执行文件目录下找 models/
    const QString baseDir = QCoreApplication::applicationDirPath();
    const QString modelsDir = QDir(baseDir).filePath("models");
    const QString faceXmlPath = QDir(modelsDir).filePath("haarcascade_frontalface_default.xml");
    const QString onnxPath = QDir(modelsDir).filePath("emotion-ferplus-8.onnx");

    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load(faceXmlPath.toStdString())) {
        emit errorOccurred(QString("Failed to load face detector: %1").arg(faceXmlPath));
        running_ = false;
        return;
    }

    cv::dnn::Net net;
    try {
        net = cv::dnn::readNetFromONNX(onnxPath.toStdString());
    } catch (const cv::Exception &e) {
        emit errorOccurred(QString("Failed to load emotion ONNX model: %1\n%2").arg(onnxPath, e.what()));
        running_ = false;
        return;
    }
    if (net.empty()) {
        emit errorOccurred(QString("Failed to load emotion ONNX model: %1").arg(onnxPath));
        running_ = false;
        return;
    }

    cv::VideoCapture cap(cameraIndex, cv::CAP_ANY);
    if (!cap.isOpened()) {
        emit errorOccurred(QString("Failed to open camera index=%1").arg(cameraIndex));
        running_ = false;
        return;
    }

    // 降低延迟（不保证所有后端都生效）
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    QElapsedTimer fpsTimer;
    fpsTimer.start();
    int frameCount = 0;
    double lastFps = 0.0;

    while (running_) {
        cv::Mat frame;
        if (!cap.read(frame) || frame.empty())
            continue;

        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(gray, faces, 1.2, 5, 0, cv::Size(60, 60));

        QString bestLabel = "NoFace";
        double bestConf = 0.0;
        cv::Rect bestFace;

        // 多人脸：取面积最大的作为“主脸”
        for (const auto &face : faces) {
            if (face.area() > bestFace.area())
                bestFace = face;
        }

        if (bestFace.area() > 0) {
            // 1. 给脸部框增加 Padding，向外扩展 15%，包含完整的面部特征
            int padX = bestFace.width * 0.15;
            int padY = bestFace.height * 0.15;
            cv::Rect paddedFace;
            paddedFace.x = std::max(0, bestFace.x - padX);
            paddedFace.y = std::max(0, bestFace.y - padY);
            paddedFace.width = std::min(gray.cols - paddedFace.x, bestFace.width + 2 * padX);
            paddedFace.height = std::min(gray.rows - paddedFace.y, bestFace.height + 2 * padY);

            // 截取扩充后的人脸区域
            cv::Mat faceROI = gray(paddedFace).clone();

            // 2. 预处理：直接用 blobFromImage 进行缩放，去除原先缩小255倍的操作
            // ferplus 期望输入通常为 N x 1 x 64 x 64
            cv::Mat blob = cv::dnn::blobFromImage(faceROI, 1.0, cv::Size(64, 64),
                                                  cv::Scalar(0), false, false, CV_32F);

            net.setInput(blob);
            cv::Mat prob = net.forward(); // 1x8 输出未激活的 Logits

            // 3. 计算 Softmax，将 Logits 转化为真实的 0~1 的置信度概率
            cv::Mat probExp;
            cv::exp(prob, probExp);
            cv::Scalar sumExp = cv::sum(probExp);
            cv::Mat softmaxProb = probExp / sumExp[0]; // 归一化为概率

            cv::Point classIdPoint;
            double maxVal = 0.0;
            cv::minMaxLoc(softmaxProb, nullptr, &maxVal, nullptr, &classIdPoint);
            
            int idx = classIdPoint.x;
            if (idx < 0) idx = 0;
            if (idx > 7) idx = 7;

            bestLabel = QString::fromLatin1(kEmotionLabels[idx]);
            bestConf = maxVal; // 此时的置信度是标准概率

            // 在画面上绘制框和文字（依然使用最初的 bestFace 进行绘制，防止视觉上绿框过大）
            cv::rectangle(frame, bestFace, cv::Scalar(0, 255, 0), 2);
            const std::string text = (bestLabel.toStdString() + " " + cv::format("%.2f", bestConf));
            cv::putText(frame, text, cv::Point(bestFace.x, std::max(0, bestFace.y - 8)),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 0, 0), 2);
        }

        // FPS 叠字
        frameCount++;
        if (fpsTimer.elapsed() >= 1000) {
            lastFps = frameCount * 1000.0 / fpsTimer.elapsed();
            frameCount = 0;
            fpsTimer.restart();
        }
        cv::putText(frame, cv::format("FPS %.1f", lastFps), cv::Point(10, 25),
                    cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);

        emit emotionUpdated(bestLabel, bestConf);
        emit frameUpdated(matToQImageBgr(frame));

        // 简单限速，避免 CPU 拉满
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}