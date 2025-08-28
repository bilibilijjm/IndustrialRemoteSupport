#include "ChatWidget.h"
#include "ui_ChatWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QScrollBar>

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatWidget)
    , chatList(new QListWidget(this))
    , messageEdit(new QTextEdit(this))
    , sendButton(new QPushButton(tr("发送"), this))
    , clearButton(new QPushButton(tr("清空"), this))
{
    ui->setupUi(this);
    setupUI();
}

ChatWidget::~ChatWidget()
{
    delete ui;
}

void ChatWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 聊天记录列表
    chatList->setAlternatingRowColors(true);
    chatList->setStyleSheet("QListWidget { border: 1px solid #2a3b5a; border-radius: 4px; }");
    mainLayout->addWidget(chatList);

    // 消息输入区域
    QHBoxLayout *inputLayout = new QHBoxLayout();

    messageEdit->setPlaceholderText(tr("输入消息..."));
    messageEdit->setMaximumHeight(80);
    connect(messageEdit, &QTextEdit::textChanged, this, &ChatWidget::updateSendButtonState);

    sendButton->setEnabled(false);
    connect(sendButton, &QPushButton::clicked, this, &ChatWidget::sendMessage);

    clearButton->setEnabled(true);
    connect(clearButton, &QPushButton::clicked, this, &ChatWidget::clearChat);

    inputLayout->addWidget(messageEdit, 4);
    inputLayout->addWidget(sendButton, 1);
    inputLayout->addWidget(clearButton, 1);

    mainLayout->addLayout(inputLayout);
    setLayout(mainLayout);
}

void ChatWidget::sendMessage()
{
    QString message = messageEdit->toPlainText().trimmed();
    if (message.isEmpty()) return;

    emit messageSent(message);

    // 添加到聊天记录（自己的消息）
    addMessage("我", message, QDateTime::currentDateTime(), true);

    messageEdit->clear();
}

void ChatWidget::receiveMessage(const QString &user, const QString &message, const QDateTime &time)
{
    addMessage(user, message, time, false);
}

void ChatWidget::addMessage(const QString &user, const QString &message, const QDateTime &time, bool isOwn)
{
    QListWidgetItem *item = new QListWidgetItem();

    QString formattedMessage = QString("[%1] %2: %3")
                               .arg(time.toString("hh:mm:ss"))
                               .arg(user)
                               .arg(message);

    item->setText(formattedMessage);

    // 设置不同样式区分自己和他人的消息
    if (isOwn) {
        item->setBackground(QColor(40, 60, 100, 100)); // 蓝色背景
        item->setTextAlignment(Qt::AlignRight);
    } else {
        item->setBackground(QColor(60, 40, 100, 100)); // 紫色背景
        item->setTextAlignment(Qt::AlignLeft);
    }

    chatList->addItem(item);

    // 自动滚动到底部
    chatList->scrollToBottom();
}

void ChatWidget::clearChat()
{
    chatList->clear();
}

void ChatWidget::updateSendButtonState()
{
    sendButton->setEnabled(!messageEdit->toPlainText().trimmed().isEmpty());
}
