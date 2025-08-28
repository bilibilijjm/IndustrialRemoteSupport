#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    QString getUsername() const;
    QString getPassword() const;
    bool isRegistering() const;

signals:
    void loginRequested(const QString &username, const QString &password);
    void registerRequested(const QString &username, const QString &password);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void toggleMode();

private:
    void setupUI();
    bool validateInput();

private:
    Ui::LoginDialog *ui;
    QLineEdit *usernameEdit;
    QLineEdit *passwordEdit;
    QPushButton *actionButton;
    QPushButton *switchModeButton;
    QLabel *titleLabel;

    bool isLoginMode;
    QString username;
    QString password;
};

#endif // LOGINDIALOG_H
