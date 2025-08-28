#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QJsonObject>

QT_BEGIN_NAMESPACE
namespace Ui { class ChatWidget; }
QT_END_NAMESPACE

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(QWidget *parent = nullptr);
    ~ChatWidget();
    void addMessage(const QString &user, const QString &message, const QDateTime &time, bool isOwn);

signals:
    void messageSent(const QString &message);

public slots:
    void receiveMessage(const QString &user, const QString &message, const QDateTime &time);
    void clearChat();

private slots:
    void sendMessage();
    void updateSendButtonState();

private:
    void setupUI();

private:
    Ui::ChatWidget *ui;
    QListWidget *chatList;
    QTextEdit *messageEdit;
    QPushButton *sendButton;
    QPushButton *clearButton;

    QString currentUser;
};

#endif // CHATWIDGET_H
