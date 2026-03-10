把下面两个文件放到本目录（与 exe 同级的 models/）：

1) 人脸检测器（OpenCV 自带）：
   - haarcascade_frontalface_default.xml

2) 表情识别 ONNX 模型（FERPlus，8 类表情）：
   - emotion-ferplus-8.onnx

运行程序时，EmotionAnalyzer 会按如下路径加载：
   <程序所在目录>\models\haarcascade_frontalface_default.xml
   <程序所在目录>\models\emotion-ferplus-8.onnx

提示：
- 如果你用 Qt Creator 运行，exe 一般在 build 目录里，请把 models/ 放到 exe 旁边。
- 也可以在 Qt Creator 的 Run 设置里把 “Working directory” 指到包含 models/ 的目录。

