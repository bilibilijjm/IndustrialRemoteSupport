#include "VideoWidget.h"
#include "ui_VideoWidget.h"
#include <QBoxLayout>
#include <QMessageBox>
#include <QScreen>
#include <QGuiApplication>
#include <QDateTime>
#include <QCameraInfo>
#include <QTimer>

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoWidget)
    , camera(nullptr)
    , cameraView(nullptr)
    , mediaRecorder(nullptr)
    , audioInput(nullptr)
    , videoProbe(nullptr)
    , currentRecordingFile(QString())
    , isVideoActive(false)
    , isAudioActive(false)
    , isRecording(false)
    , isScreenSharing(false)
{
    ui->setupUi(this);
    setupUI();
    setupCamera();
}

VideoWidget::~VideoWidget()
{
    cleanupCamera();
    delete ui;
}

void VideoWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 视频显示区域
    cameraView = new QCameraViewfinder(this);
    cameraView->setMinimumSize(640, 480);
    mainLayout->addWidget(cameraView);

    // 控制按钮区域
    QHBoxLayout *controlLayout = new QHBoxLayout();

    QPushButton *videoBtn = new QPushButton(tr("启动视频"), this);
    videoBtn->setCheckable(true);
    connect(videoBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) startVideo();
        else stopVideo();
    });

    QPushButton *audioBtn = new QPushButton(tr("启动音频"), this);
    audioBtn->setCheckable(true);
    connect(audioBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) startAudio();
        else stopAudio();
    });

    QPushButton *screenBtn = new QPushButton(tr("屏幕共享"), this);
    screenBtn->setCheckable(true);
    connect(screenBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) startScreenShare();
        else stopScreenShare();
    });

    QPushButton *recordBtn = new QPushButton(tr("开始录制"), this);
    recordBtn->setCheckable(true);
    connect(recordBtn, &QPushButton::toggled, this, [this](bool checked) {
        if (checked) startRecording();
        else stopRecording();
    });

    controlLayout->addWidget(videoBtn);
    controlLayout->addWidget(audioBtn);
    controlLayout->addWidget(screenBtn);
    controlLayout->addWidget(recordBtn);
    controlLayout->addStretch();

    mainLayout->addLayout(controlLayout);
    setLayout(mainLayout);
}

void VideoWidget::setupCamera()
{
    // 获取默认摄像头
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    if (cameras.isEmpty()) {
        ui->statusLabel->setText(tr("未检测到摄像头"));
        return;
    }

    camera = new QCamera(cameras.first(), this);
    cameraView = new QCameraViewfinder(this);
    camera->setViewfinder(cameraView);

    mediaRecorder = new QMediaRecorder(camera, this);

    connect(camera, QOverload<QCamera::Error>::of(&QCamera::error),
            this, &VideoWidget::displayCameraError);
    connect(mediaRecorder, &QMediaRecorder::stateChanged,
            this, &VideoWidget::updateRecorderState);
    connect(mediaRecorder, QOverload<QMediaRecorder::Error>::of(&QMediaRecorder::error),
            this, &VideoWidget::displayRecorderError);
}

void VideoWidget::cleanupCamera()
{
    // 停止录制器
    if (mediaRecorder) {
        if (mediaRecorder->state() != QMediaRecorder::StoppedState) {
            mediaRecorder->stop();
        }
        mediaRecorder->deleteLater();
        mediaRecorder = nullptr;
    }

    // 停止摄像头
    if (camera) {
        camera->stop();
        camera->unload();

        camera->deleteLater();
        camera = nullptr;
    }
}

QString VideoWidget::getCurrentRecordingFile() const
{
    return currentRecordingFile;
}

void VideoWidget::startVideo()
{
    if (camera) {
        camera->start();
        isVideoActive = true;
        emit videoStateChanged(true);
    }
}

void VideoWidget::stopVideo()
{
    if (camera) {
        camera->stop();
        isVideoActive = false;
        emit videoStateChanged(false);
    }
}

void VideoWidget::startAudio()
{
    // 需要其他成员实现音频采集和传输
    isAudioActive = true;
    emit audioStateChanged(true);
}

void VideoWidget::stopAudio()
{
    isAudioActive = false;
    emit audioStateChanged(false);
}

void VideoWidget::startScreenShare()
{
    // 屏幕共享功能需要其他成员实现
    isScreenSharing = true;
    emit screenShareStateChanged(true);
    QMessageBox::information(this, tr("信息"), tr("屏幕共享功能需要其他成员实现"));
}

void VideoWidget::stopScreenShare()
{
    isScreenSharing = false;
    emit screenShareStateChanged(false);
}

void VideoWidget::startRecording()
{
    if (!camera || !mediaRecorder) {
        qWarning() << "无法开始录制：摄像头或录制器未初始化";
        return;
    }

    try {
        // 确保录制目录存在
        QDir recordDir("recordings");
        if (!recordDir.exists()) {
            if (!recordDir.mkpath(".")) {
                qWarning() << "无法创建录制目录";
                QMessageBox::warning(this, tr("错误"), tr("无法创建录制目录"));
                return;
            }
        }

        // 生成唯一的文件名
        currentRecordingFile = QString("recordings/recording_%1_%2.mp4")
                              .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"))
                              .arg(qrand() % 1000); // 添加随机数避免重名

        // 设置输出位置
        mediaRecorder->setOutputLocation(QUrl::fromLocalFile(currentRecordingFile));

        // 开始录制
        mediaRecorder->record();
        isRecording = true;
        emit recordingStateChanged(true);

        // 调试信息
        qInfo() << "录制已开始，保存至:" << currentRecordingFile;

    } catch (const std::exception &e) {
        // 错误处理
        qCritical() << "录制启动失败:" << e.what();
        QMessageBox::warning(this, tr("录制错误"),
                            tr("无法开始录制: %1").arg(e.what()));
        currentRecordingFile.clear();
    }
}

void VideoWidget::stopRecording()
{
    if (mediaRecorder) {
        // 停止逻辑
        mediaRecorder->stop();

        // 状态检查和调试信息
        if (mediaRecorder->state() == QMediaRecorder::RecordingState) {
            // 如果还在录制状态，等待一下再检查
            QTimer::singleShot(100, this, [this]() {
                isRecording = false;
                emit recordingStateChanged(false);
                qInfo() << "录制已停止，文件保存至:" << currentRecordingFile;

                // 文件验证逻辑
                if (!currentRecordingFile.isEmpty()) {
                    QFile file(currentRecordingFile);
                    if (file.exists()) {
                        qDebug() << "录制文件大小:" << file.size() << "字节";
                    }
                }
            });
        } else {
            isRecording = false;
            emit recordingStateChanged(false);
            qInfo() << "录制已停止，文件保存至:" << currentRecordingFile;
        }
    } else {
        isRecording = false;
        emit recordingStateChanged(false);
    }
}

void VideoWidget::toggleRecording()
{
    if (isRecording) {
        stopRecording();
    } else {
        startRecording();
    }
}

void VideoWidget::updateCameraState(QCamera::State state)
{
    switch (state) {
    case QCamera::ActiveState:
        ui->statusLabel->setText(tr("摄像头已启动"));
        break;
    case QCamera::UnloadedState:
    case QCamera::LoadedState:
        ui->statusLabel->setText(tr("摄像头就绪"));
        break;
    }
}

void VideoWidget::updateRecorderState(QMediaRecorder::State state)
{
    switch (state) {
    case QMediaRecorder::RecordingState:
        ui->statusLabel->setText(tr("正在录制..."));
        break;
    case QMediaRecorder::PausedState:
        ui->statusLabel->setText(tr("录制暂停"));
        break;
    case QMediaRecorder::StoppedState:
        ui->statusLabel->setText(tr("录制已停止"));
        break;
    }
}

void VideoWidget::displayRecorderError()
{
    QMessageBox::warning(this, tr("录制错误"), mediaRecorder->errorString());
}

void VideoWidget::displayCameraError()
{
    QMessageBox::warning(this, tr("摄像头错误"), camera->errorString());
}
