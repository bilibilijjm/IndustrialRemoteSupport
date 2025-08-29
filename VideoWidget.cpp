#include "VideoWidget.h"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QVBoxLayout>
#include <QFile>
#include <QUrl>

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    view_ = new QWebEngineView(this);
    layout->addWidget(view_);

    grantMediaPermissions();
}

void VideoWidget::grantMediaPermissions()
{
    // 自动允许音视频采集（开发期方便；正式可弹窗让用户确认）
    connect(view_->page(), &QWebEnginePage::featurePermissionRequested,
            this, [this](const QUrl &origin, QWebEnginePage::Feature f){
        if (f == QWebEnginePage::MediaAudioCapture ||
            f == QWebEnginePage::MediaVideoCapture ||
            f == QWebEnginePage::MediaAudioVideoCapture) {
            view_->page()->setFeaturePermission(origin, f, QWebEnginePage::PermissionGrantedByUser);
        } else {
            view_->page()->setFeaturePermission(origin, f, QWebEnginePage::PermissionDeniedByUser);
        }
    });
}

void VideoWidget::startCall(const QString& roomId, const QString& display, const QString& janusUrl)
{
    QFile f(":/web/janus_client.html");
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QString html = QString::fromUtf8(f.readAll());
    html.replace("${JANUS_URL}", janusUrl);
    html.replace("${ROOM_ID}",  roomId);
    html.replace("${DISPLAY}",  display);

    // baseUrl 指向资源前缀，方便今后引用相对路径资源
    view_->setHtml(html, QUrl("qrc:/web/"));
}
