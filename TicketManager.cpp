#include "TicketManager.h"
#include "ui_TicketManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDateTime>
#include <QMessageBox>
TicketManager::TicketManager(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TicketManager)
    , ticketTree(new QTreeWidget(this))
    , createButton(new QPushButton(tr("新建工单"), this))
    , joinButton(new QPushButton(tr("加入工单"), this))
    , refreshButton(new QPushButton(tr("刷新"), this))
    , ticketTitleEdit(new QLineEdit(this))
{
    ui->setupUi(this);
    setupUI();
    loadTickets();
}

TicketManager::~TicketManager()
{
    delete ui;
}

void TicketManager::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 工单树形列表
    ticketTree->setHeaderLabels(QStringList() << tr("工单ID") << tr("标题") << tr("状态") << tr("创建时间"));
    ticketTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ticketTree->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(ticketTree, &QTreeWidget::itemSelectionChanged, this, &TicketManager::onTicketSelectionChanged);

    mainLayout->addWidget(ticketTree);

    // 输入和按钮区域
    QHBoxLayout *inputLayout = new QHBoxLayout();
    ticketTitleEdit->setPlaceholderText(tr("输入工单标题"));
    inputLayout->addWidget(new QLabel(tr("标题:"), this));
    inputLayout->addWidget(ticketTitleEdit, 2);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    createButton->setEnabled(false);
    connect(createButton, &QPushButton::clicked, this, &TicketManager::onCreateNewTicket);

    joinButton->setEnabled(false);
    connect(joinButton, &QPushButton::clicked, this, &TicketManager::onJoinSelectedTicket);

    refreshButton->setEnabled(true);
    connect(refreshButton, &QPushButton::clicked, this, &TicketManager::onRefreshTickets);

    buttonLayout->addWidget(createButton);
    buttonLayout->addWidget(joinButton);
    buttonLayout->addWidget(refreshButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    // 连接输入框变化信号
    connect(ticketTitleEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        createButton->setEnabled(!text.trimmed().isEmpty());
    });
}

void TicketManager::loadTickets()
{
    // 需要其他成员实现从数据库加载工单列表
    // 这里只是示例数据
    ticketTree->clear();
    QTreeWidgetItem *item =new QTreeWidgetItem(ticketTree);
    QSqlQuery query(DatabaseManager::instance().db());
    if (!query.exec("SELECT ticket_id, title, status, create_time FROM tickets ORDER BY create_time DESC")){
        qDebug()<<"error: can't load tickets info -"<<query.lastError().text();
        return;
    }
    while (query.next(){
        QTreeWidgetItem *item = new QTreeWidgetItem(ticketTree);
        item ->setText(0,query.value(0).toString());//ticket_id
        item ->setText(1,query.value(1).toString());//ticket_title
        item ->setText(2,query.value(2).toString());//status
        item ->setText(3,query.value(3).toString());//create_time
    })
}   

void TicketManager::onCreateNewTicket()
{
    QString title = ticketTitleEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请输入工单标题"));
        return;
    }
    QSqlQuery check(DatabaseManager::instance().db());
    check.prepare("SELECT COUNT(*) FROM tickets WHERE title =?");
    check.addBindValue(title);
    check.exec();
    if(check.next() && check.value(0).toInt()>0){
        QMessageBox::warning (this, tr("提示"),tr("已存在相同标题的工单"));
    }
    QString ticketId = "T" + QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    saveTicket(ticketId, title);

    emit createTicket();
    emit ticketCreated(ticketId);

    ticketTitleEdit->clear();
    loadTickets(); // 刷新列表
}

void TicketManager::onJoinSelectedTicket()
{
    if (ticketTree->currentItem()) {
        emit joinTicket(getSelectedTicketId());
    }
}

void TicketManager::onRefreshTickets()
{
    loadTickets();
}

void TicketManager::onTicketSelectionChanged()
{
    joinButton->setEnabled(ticketTree->currentItem() != nullptr);
}

QString TicketManager::getSelectedTicketId() const
{
    if (ticketTree->currentItem()) {
        return ticketTree->currentItem()->text(0);
    }
    return QString();
}

void TicketManager::addTicket(const QString &ticketId)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(ticketTree);
    item->setText(0, ticketId);
    item->setText(1, tr("新工单"));
    item->setText(2, tr("待处理"));
    item->setText(3, QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    ticketTree->addTopLevelItem(item);
}

void TicketManager::updateTicketStatus(const QString &ticketId, const QString &status)
{
    for (int i = 0; i < ticketTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem *item = ticketTree->topLevelItem(i);
        if (item->text(0) == ticketId) {
            item->setText(2, status);
            break;
        }
    }
    QSqlQuery query(DatabaseManager::instance().db());
    query.prepare("UPDATE tickets SET status = ? WHERE ticket_d= ? ");
    query.addBindValue(status);
    query.addBindValue(ticketId);
    if(!query.exec()){
        qDebug()<<"error: can't update ticket's status - "<<query.lastError().text();
    }
}

void TicketManager::saveTicket(const QString &ticketId, const QString &title)
{
    QDateTime now =QDateTime::currentDateTime();
    if(!DatabaseManager::instance()).saveTicket(ticketId, title, "", now){
        QMessageBox::critical(this,tr("错误"),tr("保存工单失败"));
    }
}
