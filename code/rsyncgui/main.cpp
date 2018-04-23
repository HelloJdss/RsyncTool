#include <QApplication>
#include <QDateTime>
#include <QPixmap>
#include <QSplashScreen>
#include <QDesktopWidget>
#include <QMessageBox>

#include "mainwindow.h"
#include "MainMod.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    QApplication::setQuitOnLastWindowClosed(false); //后台运行时不一定具备可视化主窗口，此时不应该退出

    QSplashScreen screen(QPixmap(":/image/res/image1.png"));
    screen.show();

    QTime timer;
    timer.start();
    MainWindow w;


    //TODO: 加载配置文件
    if(!g_MainMod->LoadDataOnStart(&screen))
    {
        //screen.showMessage(g_MainMod->GetLastErr(), Qt::AlignLeft | Qt::AlignBottom);
        QMessageBox::critical(nullptr, QStringLiteral("错误！"), g_MainMod->GetLastErr());
        return 0;
    }

    do
    {
        QApplication::processEvents();

    }while (timer.elapsed() < 1000);//1为需要延时的秒数

    w.show();

    screen.finish(&w);

    return QApplication::exec();
}
