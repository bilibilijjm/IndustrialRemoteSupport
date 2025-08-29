#include "DatabaseManager.h"

DatabaseManager& DatabaseManager::instance(){
    static DatabaseManager instance;
    reuturn instance;
}
DatabaseManager:: DatabaseManager(QObject *parent)
    :QObject(parent){}
DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
}    
bool DatabaseManager::initialize()
{
    // 直接赋值给成员变量 db，而不是新建局部变量
    this->db = QSqlDatabase::addDatabase("QSQLITE");
    this->db.setDatabaseName("Industrial_Remote_Control.db");

    if (!this->db.open()) {
        qDebug() << "connect error!" << this->db.lastError().text();
        return false;
    }

    // 创建 QSqlQuery 时绑定到同一个连接
    QSqlQuery query(this->db);

    // 用户表
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS users("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL)"
    )) {
        qDebug() << "ERROR: FAIL TO CREATE TABLE users" << query.lastError().text();
        return false;
    }

    // 工单表
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS tickets("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "ticket_id TEXT UNIQUE NOT NULL,"
        "title TEXT,"
        "description TEXT,"
        "create_time DATETIME)"
    )) {
        qDebug() << "ERROR: FAIL TO CREATE TABLE tickets" << query.lastError().text();
        return false;
    }

    // 消息表
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS messages("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "ticket_id TEXT NOT NULL,"
        "user TEXT,"
        "message TEXT,"
        "time DATETIME,"
        "FOREIGN KEY(ticket_id) REFERENCES tickets(ticket_id))"
    )) {
        qDebug() << "ERROR: FAIL TO CREATE TABLE messages" << query.lastError().text();
        return false;
    }

    // 控制记录表
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS control("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "ticket_id TEXT NOT NULL,"
        "message TEXT,"
        "command TEXT,"
        "time DATETIME,"
        "FOREIGN KEY(ticket_id) REFERENCES tickets(ticket_id))"
    )) {
        qDebug() << "ERROR: FAIL TO CREATE TABLE control" << query.lastError().text();
        return false;
    }

    // 视频记录表
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS video("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "ticket_id TEXT NOT NULL,"
        "file_location TEXT UNIQUE NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(ticket_id) REFERENCES tickets(ticket_id))"
    )) {
        qDebug() << "ERROR: FAIL TO CREATE TABLE video" << query.lastError().text();
        return false;
    }

    // 音频记录表
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS voice_call("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "ticket_id TEXT NOT NULL,"
        "file_location TEXT UNIQUE NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY(ticket_id) REFERENCES tickets(ticket_id))"
    )) {
        qDebug() << "ERROR: FAIL TO CREATE TABLE voice_call" << query.lastError().text();
        return false;
    }

    return true;
}
bool DatabaseManager::addUser(const QString &username, const QString &password){
    QSqlQuery query(this->db);
    query.prepare("INSERT INTO users(username, password) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);//将参数username 和 password 依次绑定到两个？占位符上
    if (!query.exec()) {
    qDebug() << "addUser error:" << query.lastError().text();
    return false;
    }
    return true;
}
bool DatabaseManager::verifyUser(const QString &username, const QString &password)
{
    QSqlQuery query(this->db);
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(password);

    if (!query.exec()) {
        qDebug() << "verifyUser error:" << query.lastError().text();
        return false;
    }

    if (query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}
bool DatabaseManager::saveTicket(const QString &ticketId, const QString &title,const QString &description, const QDateTime &createTime)
{
    QSqlQuery query(this->db);
    query.prepare("INSERT INTO tickets (ticket_id, title, description, create_time) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(ticketId);
    query.addBindValue(title);
    query.addBindValue(description);
    query.addBindValue(createTime.toString(Qt::ISODate));

    if (!query.exec()) {
        qDebug() << "saveTicket error:" << query.lastError().text();
        return false;
    }
    return true;
}
bool DatabaseManager::saveMessage(const QString &ticketId, const QString &user, const QString &message, const QDateTime &time)
{
    QSqlQuery query(this->db);
    query.prepare("INSERT INTO messages (ticket_id, user, message, time) "
                  "VALUES (?, ?, ?, ?)");
    query.addBindValue(ticketId);
    query.addBindValue(user);
    query.addBindValue(message);
    query.addBindValue(time.toString(Qt::ISODate));

    if (!query.exec()) {
        qDebug() << "saveMessage error:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::saveRecording(const QString &ticketId, const QString &filePath, const QDateTime &startTime, const QDateTime &endTime)
{
    QSqlQuery query(this->db);
    query.prepare("INSERT INTO voice_call (ticket_id, file_location, created_at) "
                  "VALUES (?, ?, ?)");
    query.addBindValue(ticketId);
    query.addBindValue(filePath);
    query.addBindValue(startTime.toString(Qt::ISODate)); // 或者用 endTime，根据需求

    if (!query.exec()) {
        qDebug() << "saveRecording error:" << query.lastError().text();
        return false;
    }
    return true;
}