#ifndef TICKETMANAGER_H
#define TICKETMANAGER_H

#include <QWidget>
#include <QTreeWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QJsonObject>
#include <QLabel>
#include "DatabaseManager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class TicketManager; }
QT_END_NAMESPACE

class TicketManager : public QWidget
{
    Q_OBJECT

public:
    explicit TicketManager(QWidget *parent = nullptr);
    ~TicketManager();

    QString getSelectedTicketId() const;
    void addTicket(const QString &ticketId);
    void updateTicketStatus(const QString &ticketId, const QString &status);

signals:
    void createTicket();
    void joinTicket();
    void ticketCreated(const QString &ticketId);

private slots:
    void onCreateNewTicket();
    void onJoinSelectedTicket();
    void onRefreshTickets();
    void onTicketSelectionChanged();

private:
    void setupUI();
    void loadTickets(); // 需要其他成员实现从数据库加载
    void saveTicket(const QString &ticketId, const QString &title); // 需要其他成员实现保存到数据库

private:
    Ui::TicketManager *ui;
    QTreeWidget *ticketTree;
    QPushButton *createButton;
    QPushButton *joinButton;
    QPushButton *refreshButton;
    QLineEdit *ticketTitleEdit;
};

#endif // TICKETMANAGER_H
