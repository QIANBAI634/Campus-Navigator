/**
 * 校园导航系统 - 主入口
 * 基于数据结构课程设计：北邮沙河校区导航
 * 从 HTML/JS 版本翻译为 C++ Qt 实现
 */
#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序元数据
    app.setApplicationName("校园导航系统");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("BUPT");

    // 全局字体设置（使中文字体更好看）
    QFont globalFont = app.font();
    globalFont.setPointSize(10);
    app.setFont(globalFont);

    MainWindow w;
    w.show();

    return app.exec();
}
