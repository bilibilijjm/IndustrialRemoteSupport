#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 设置应用程序信息
    QApplication::setApplicationName("IndustrialRemoteSupport");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("IndustrialTech");

    // 加载科技风格字体
    int fontId = QFontDatabase::addApplicationFont(":/fonts/RobotoMono-Regular.ttf");
    if (fontId != -1) {
        QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
        QFont font(family, 10);
        QApplication::setFont(font);
    }

    MainWindow w;
    w.show();

    return a.exec();
}
