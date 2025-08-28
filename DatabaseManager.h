// DatabaseManager.h - 需要其他成员实现
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance();

    bool initialize();
    bool addUser(const QString &username, const QString &password);
    bool verifyUser(const QString &username, const QString &password);
    bool saveTicket(const QString &ticketId, const QString &title,
                   const QString &description, const QDateTime &createTime);
    bool saveMessage(const QString &ticketId, const QString &user,
                    const QString &message, const QDateTime &time);
    bool saveRecording(const QString &ticketId, const QString &filePath,
                      const QDateTime &startTime, const QDateTime &endTime);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
