#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QWidget>
#include <QCamera>
#include <QCameraViewfinder>
#include <QAudioInput>
#include <QMediaRecorder>
#include <QVideoProbe>
#include <QPushButton>
#include <QLabel>
#include <QUrl>
#include <QDir>

QT_BEGIN_NAMESPACE
namespace Ui { class VideoWidget; }
QT_END_NAMESPACE

class VideoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoWidget(QWidget *parent = nullptr);
    ~VideoWidget();
    QString getCurrentRecordingFile() const;

signals:
    void videoStateChanged(bool started);
    void audioStateChanged(bool started);
    void screenShareStateChanged(bool started);
    void recordingStateChanged(bool started);

public slots:
    void startVideo();
    void stopVideo();
    void startAudio();
    void stopAudio();
    void startScreenShare();
    void stopScreenShare();
    void startRecording();
    void stopRecording();
    void toggleRecording();

private slots:
    void updateCameraState(QCamera::State state);
    void updateRecorderState(QMediaRecorder::State state);
    void displayRecorderError();
    void displayCameraError();

private:
    void setupUI();
    void setupCamera();
    void cleanupCamera();

private:
    Ui::VideoWidget *ui;
    QCamera *camera;
    QCameraViewfinder *cameraView;
    QMediaRecorder *mediaRecorder;
    QAudioInput *audioInput;
    QVideoProbe *videoProbe;
    QString currentRecordingFile;

    bool isVideoActive;
    bool isAudioActive;
    bool isRecording;
    bool isScreenSharing;
};

#endif // VIDEOWIDGET_H
