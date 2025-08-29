// DatabaseManager.h - 需要其他成员实现
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>
#include <QDateTime>

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager& instance();
    bool initialize(){
        QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("Industrial_Remote_Control");
        if (!db.open()){
            qDebug()<<"connect error!"<<db.lastError().text();
            return false;
        }
        QSqlQuery query;
        if(!query.exec(
            "CREATE TABLE IF NOT EXISTS users("
            "id IINTERGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT UNIQUE NOT NULL,"
            "password TEXT NOT NULL)"//可以在此处增加对用户名和密码的约束。
        )){
            qDebug()<<"ERROR: FAIL TO CREATE TABLE user"<<query.lastError().text();
            return false;
        }//用户表
        if(!query.exec(
            "CREATE TABLE IF NOT EXISTS tickets("
            "id INTERGER PRIMARY KEY AUTOINCREMENT,"
            "ticket_id TEXT UNIQUE NOT NULL,"
            "titile TEXT,"
            "description TEXT,"
            "create_time DATETIME"
        )){
            qDebug()<<"ERROR: FAIL TO CREATE TABLE tickets"<<query.lastError().text();
            return false;
        }//工单表
        if(!query.exec(
            "CREATE TABLE IF NOT EXISTS messages ("
            "id INTERGER PRIMARY KEY AUTOINCREMENT,"
            "ticket_id TEXT NOT NULL,"
            "user TEXT,"
            "message TEXT,"
            "time DATETIME,"
            "FOREIGN KEY(ticket_id) REFERENCES tickets(ticket_id))"
        )){
            qDebug()<<"ERROR: FAIL TO CREATE TABLE message"<<query.lastError().text();
            return false;
        }//信息表
        if(!query.exec(
            "CREATE TABLE IF NOT EXISTS control"
            "id INTERGER PRIMARY KEY AUTOINCREMENT,"
            "ticket_id TEXT NOT NULL,"
            "message TEXT,"
            "command TEXT,"
            "time DATETIME,"
            "FOREIGN KEY(ticket_id) REFERENCES tickets(ticket_id)"
        )){
            qDebug()<<"ERROR: FAIL TO CREATE TABLE cotrol"<<query.lastError().text();
            return false;
        }//控制记录表
        if(!query.exec(
            "CREATE TABLE IF NOT EXISTS video"
            "id INTERGER PRIMARY KEY AUTOINCREMENT,"
            "ticket_id TEXT NOT NULL,"
            "file_location TEXT UNIQUE NOT NULL"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        )){
            qDebug()<<"ERROR: FAIL TO CREATE TABLE video"<<query.lastError().text();
            return false;
        }//视频记录
        if(!query.exec(
            "CREATE TABLE IF NOT EXISTS voice_call"
            "id INTERGER PRIMARY KEY AUTOINCREMENT,"
            "ticket_id TEXT NOT NULL,"
            "file_location TEXT UNIQUE NOT NULL"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
        )){
            qDebug()<<"ERROR: FAIL TO CREATE TABLE voice_call"<<query.lastError().text();
            return false;
        }//音频记录
        return true;
    }
    bool addUser(const QString &username, const QString &password){
        
    };

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
