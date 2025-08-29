#include "ChatWidget.h"
#include "ui_ChatWidget.h"
#include "UdpBus.h"

#include <QHostAddress>
#include <QJsonObject>
#include <QListWidget>
#include <QAction>
#include <QKeySequence>

static const quint16      kRoomPort  = 50000;                   // 统一端口
static const QHostAddress kRoomGroup("239.10.10.10");           // 组播地址
static const QString      kChannel   = QStringLiteral("chat");  // 频道名

ChatWidget::ChatWidget(QWidget *parent)
    : QWidget(parent), ui(new Ui::ChatWidget)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("UDP Chat"));

    // UI 增强：列表样式 & 发送键状态 & 清空动作
    ui->listWidget->setAlternatingRowColors(true);
    ui->listWidget->setStyleSheet(
        "QListWidget { border: 1px solid #2a3b5a; border-radius: 4px; }");

    ui->lineEdit->setPlaceholderText(tr("输入消息..."));
    ui->sendButton->setEnabled(false);
    connect(ui->lineEdit, &QLineEdit::textChanged,
            this, &ChatWidget::updateSendButtonState);

    // 清空聊天（右键菜单 & 快捷键 Ctrl+K）
    auto clearAct = new QAction(tr("清空聊天"), this);
    clearAct->setShortcut(QKeySequence(QStringLiteral("Ctrl+K")));
    addAction(clearAct);
    ui->listWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    ui->listWidget->addAction(clearAct);
    connect(clearAct, &QAction::triggered, this, &ChatWidget::clearChat);

    // UdpBus 组播收发
    bus_ = UdpBus::instance();

    static bool bound = false;
    if (!bound) {
        bound = bus_->bind(kRoomPort, QHostAddress::AnyIPv4, /*share=*/true);
        if (bound) bus_->joinMulticast(kRoomGroup); // 同网段自动发现
    }

    // 收 JSON 消息 -> 加入聊天列表（对方样式，带时间）
    connect(bus_, &UdpBus::jsonReceived, this,
            [this](const QHostAddress& from, quint16 /*port*/, const QJsonObject& o){
        if (o.value(QStringLiteral("ch")).toString() != kChannel) return;
        const QString text = o.value(QStringLiteral("text")).toString();
        const QString user = o.value(QStringLiteral("user")).toString().trimmed().isEmpty()
                             ? from.toString()
                             : o.value(QStringLiteral("user")).toString();
        addMessage(user, text, QDateTime::currentDateTime(), /*isOwn=*/false);
    });

    // 错误提示
    connect(bus_, &UdpBus::errorOccurred, this, [this](const QString& err){
        addMessage(QStringLiteral("系统"), QStringLiteral("错误: ") + err,
                   QDateTime::currentDateTime(), /*isOwn=*/false);
    });

    // 发消息：按钮/回车
    connect(ui->sendButton, &QPushButton::clicked, this, &ChatWidget::onSendClicked);
    connect(ui->lineEdit,   &QLineEdit::returnPressed, this, &ChatWidget::onSendClicked);
}

ChatWidget::~ChatWidget() { delete ui; }

void ChatWidget::onSendClicked()
{
    if (!bus_) return;
    const QString text = ui->lineEdit->text().trimmed();
    if (text.isEmpty()) return;

    // 先本地显示（自己的消息样式，带时间）
    addMessage(displayName_, text, QDateTime::currentDateTime(), /*isOwn=*/true);

    // 组播发出去（带上 user）
    QJsonObject msg{
        { "ch",   kChannel },
        { "type", "msg"    },
        { "user", displayName_ },
        { "text", text     }
    };
    bus_->sendJson(msg, kRoomGroup, kRoomPort);

    ui->lineEdit->clear();
}

void ChatWidget::clearChat()
{
    ui->listWidget->clear();
}

void ChatWidget::updateSendButtonState()
{
    ui->sendButton->setEnabled(!ui->lineEdit->text().trimmed().isEmpty());
}

void ChatWidget::addMessage(const QString &user, const QString &message,
                            const QDateTime &time, bool isOwn)
{
    auto *item = new QListWidgetItem;
    const QString formatted = QString("[%1] %2: %3")
                                .arg(time.toString("hh:mm:ss"))
                                .arg(user)
                                .arg(message);
    item->setText(formatted);

    if (isOwn) {
        item->setBackground(QColor(40, 60, 100, 100));  // 自己：蓝色
        item->setTextAlignment(Qt::AlignRight);
    } else {
        item->setBackground(QColor(60, 40, 100, 100));  // 对方：紫色
        item->setTextAlignment(Qt::AlignLeft);
    }

    ui->listWidget->addItem(item);
    ui->listWidget->scrollToBottom();
}
