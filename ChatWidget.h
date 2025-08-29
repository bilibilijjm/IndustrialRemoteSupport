#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui { class ChatWidget; }
QT_END_NAMESPACE

class UdpBus;  // 前向声明

class ChatWidget : public QWidget {
    Q_OBJECT
public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();

    void setDisplayName(const QString& name) { displayName_ = name; }

private slots:
    void onSendClicked();
    void clearChat();
    void updateSendButtonState();

private:
    void addMessage(const QString& user, const QString& message,
                    const QDateTime& time, bool isOwn);

private:
    Ui::ChatWidget *ui = nullptr;
    UdpBus *bus_ = nullptr;
    QString displayName_ = QStringLiteral("我"); // 本端显示名
};

#endif // CHATWIDGET_H
