#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QLabel>
#include <QProcess>

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow() override;

    void closeEvent(QCloseEvent *event) override;

    void showStatusBarTip(const QString& tip);

private slots:

    void onActive_systemTray(QSystemTrayIcon::ActivationReason reason);

    void onClick_systemTrayMenu_show();

    void onClick_systemTrayMenu_exit();

    void onUpdateTime();

    void onCmdReadOutput();

    void onCmdFinished();

private slots:

    void on_action_4_triggered();   //点击最小化

    void on_action_Q_triggered();   //退出

    void on_action_5_triggered();   //新建任务

    void on_pushButton_3_clicked();   //终止当前执行的任务

private:
    Ui::MainWindow *ui;

    QSystemTrayIcon *m_systemTray = nullptr;
    QTimer *m_statusbarTimer = nullptr;
    QLabel *m_currentTimeLabel = nullptr;
    QProcess *m_cmd = nullptr;
    QString m_cmdoutput;
};

#endif // MAINWINDOW_H
