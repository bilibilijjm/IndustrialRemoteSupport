#include "LoginDialog.h"
#include "ui_LoginDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QRegularExpressionValidator>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
    , isLoginMode(true)
{
    ui->setupUi(this);
    setupUI();
    setWindowTitle(tr("工业远程支持系统 - 登录"));
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    titleLabel = new QLabel(tr("用户登录"), this);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #569cd6;");
    mainLayout->addWidget(titleLabel);

    QFormLayout *formLayout = new QFormLayout();

    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText(tr("请输入用户名"));
    usernameEdit->setValidator(new QRegularExpressionValidator(QRegularExpression("[a-zA-Z0-9_]{3,20}"), this));
    formLayout->addRow(tr("用户名:"), usernameEdit);

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText(tr("请输入密码"));
    passwordEdit->setEchoMode(QLineEdit::Password);
    passwordEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(".{6,20}"), this));
    formLayout->addRow(tr("密码:"), passwordEdit);

    mainLayout->addLayout(formLayout);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    actionButton = new QPushButton(tr("登录"), this);
    connect(actionButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);

    switchModeButton = new QPushButton(tr("注册新用户"), this);
    connect(switchModeButton, &QPushButton::clicked, this, &LoginDialog::toggleMode);

    buttonLayout->addWidget(actionButton);
    buttonLayout->addWidget(switchModeButton);

    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
    setFixedSize(400, 250);
}

void LoginDialog::onLoginClicked()
{
    if (!validateInput()) return;

    username = usernameEdit->text().trimmed();
    password = passwordEdit->text();

    if (isLoginMode) {
        emit loginRequested(username, password);
    } else {
        emit registerRequested(username, password);
    }

    accept();
}

void LoginDialog::onRegisterClicked()
{
    if (!validateInput()) return;

    username = usernameEdit->text().trimmed();
    password = passwordEdit->text();
    emit registerRequested(username, password);

    accept();
}

void LoginDialog::toggleMode()
{
    isLoginMode = !isLoginMode;

    if (isLoginMode) {
        titleLabel->setText(tr("用户登录"));
        actionButton->setText(tr("登录"));
        switchModeButton->setText(tr("注册新用户"));
    } else {
        titleLabel->setText(tr("用户注册"));
        actionButton->setText(tr("注册"));
        switchModeButton->setText(tr("返回登录"));
    }
}

bool LoginDialog::validateInput()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();

    if (username.isEmpty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("用户名不能为空"));
        return false;
    }

    if (username.length() < 3) {
        QMessageBox::warning(this, tr("输入错误"), tr("用户名长度至少3个字符"));
        return false;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, tr("输入错误"), tr("密码不能为空"));
        return false;
    }

    if (password.length() < 6) {
        QMessageBox::warning(this, tr("输入错误"), tr("密码长度至少6个字符"));
        return false;
    }

    return true;
}

QString LoginDialog::getUsername() const
{
    return username;
}

QString LoginDialog::getPassword() const
{
    return password;
}

bool LoginDialog::isRegistering() const
{
    return !isLoginMode;
}
