#include <QAction>
#include <QDialog>
#include <QMessageBox>
#include <QDebug>
#include <QDesktopWidget>
#include <QTimer>
#include <QDateTime>
#include <QFrame>
#include <QDesktopServices>
#include <QSet>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "wizard_createtask.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 2);
    ui->splitter_2->setStretchFactor(0, 1);
    ui->splitter_2->setStretchFactor(1, 4);
    ui->progressBar->hide();
    ui->pushButton_3->hide();
    ui->pushButton_4->hide();
    //ui->pushButton_2->setEnabled(false);

    m_currentTimeLabel = new QLabel(ui->statusbar);
    m_currentTimeLabel->setFrameShape(QFrame::NoFrame);
    m_statusbarTimer = new QTimer(this);
    m_statusbarTimer->start(1000); // 每次发射timeout信号时间间隔为1秒
    connect(m_statusbarTimer, SIGNAL(timeout()), this, SLOT(onUpdateTime()));
    ui->statusbar->addPermanentWidget(m_currentTimeLabel);

    this->move((QApplication::desktop()->width() - this->width()) / 2,
               (QApplication::desktop()->height() - this->height()) / 2); //居中显示

    if (QSystemTrayIcon::isSystemTrayAvailable())
    {
        m_systemTray = new QSystemTrayIcon(this);
        m_systemTray->setIcon(QIcon(":/icon/res/icon1.png"));
        m_systemTray->setToolTip(QStringLiteral("远程同步工具"));

        auto menu = new QMenu(this);
        auto act1 = new QAction(QStringLiteral("显示主界面"), this);
        connect(act1, SIGNAL(triggered()), this, SLOT(onClick_systemTrayMenu_show()));
        auto act2 = new QAction(QStringLiteral("退出"), this);
        connect(act2, SIGNAL(triggered()), this, SLOT(onClick_systemTrayMenu_exit()));
        menu->addAction(act1);
        menu->addSeparator();
        menu->addAction(act2);

        m_systemTray->setContextMenu(menu);
        connect(m_systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this,
                SLOT(onActive_systemTray(QSystemTrayIcon::ActivationReason)));
        m_systemTray->show();
    }

    g_MainMod->setMainWindow(this);

    m_taskDateModel = new QStandardItemModel(this);
    m_taskDateModel->setHorizontalHeaderLabels(QStringList() << QStringLiteral("日期"));
    ui->treeView->setModel(m_taskDateModel);

    m_cmd = new QProcess(this);
    connect(m_cmd, SIGNAL(readyRead()), this, SLOT(onCmdReadOutput()));
    connect(m_cmd, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCmdFinished()));
    m_cmd->setWorkingDirectory(QDir::currentPath());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_4_triggered() //最小化至托盘
{
    if (m_systemTray)
    {
        this->hide();
        m_systemTray->showMessage(QStringLiteral("远程同步工具"), QStringLiteral("已隐藏至托盘"));
    }
    else
    {
        this->showMinimized();
    }
}

void MainWindow::onActive_systemTray(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Trigger:
            //单击托盘图标，显示菜单
            m_systemTray->contextMenu()->show();
            break;
        case QSystemTrayIcon::DoubleClick:
            //双击托盘图标
            //双击后显示主程序窗口
            m_systemTray->contextMenu()->hide();
            this->show();
            break;
        default:
            break;
    }
}

void MainWindow::onClick_systemTrayMenu_show()
{
    this->show();
}

void MainWindow::onClick_systemTrayMenu_exit()
{
    this->close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    int ret = QMessageBox::warning(this, QStringLiteral("警告!"), QStringLiteral("你确定要退出程序吗？结束程序后，所有的任务将会终止!"),
                                   QStringLiteral("确认退出"), QStringLiteral("最小化至托盘"), QStringLiteral("取消"));
    switch (ret)
    {
        case 0:
            event->accept();
            //TODO: 做退出前的清理
            if (m_cmd->isOpen())
            {
                m_cmd->kill();
                m_cmd->waitForFinished();
            }
            g_MainMod->SaveDataOnClose();
            QApplication::quit();
            break;
        case 1:
            event->ignore();
            on_action_4_triggered();
            break;
        default:
            event->ignore();
            break;
    }
}

void MainWindow::on_action_Q_triggered()
{
    this->close();
}

void MainWindow::on_action_5_triggered()
{
    Wizard_CreateTask wizard(this);
    wizard.exec();


    const auto& task = wizard.GetTask();

    if(task.m_src.isEmpty() || task.m_des.isEmpty())
    {
        return;
    }

    g_MainMod->addTask(task);

    /*
    ui->textBrowser->clear();
    //test
    m_cmd = new QProcess(this);
    connect(m_cmd, SIGNAL(readyRead()), this, SLOT(onCmdReadOutput()));
    connect(m_cmd, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onCmdFinished()));
    //QString cmd("./rsyncclient -D -p /home/carrot/clion-2018.1.1/ /home/carrot/test/@127.0.0.1:48888");


    m_cmd->setWorkingDirectory(QDir::currentPath());
    //m_cmd->start(cmd);
    //m_cmd->start("ping 127.0.0.1");
    ui->progressBar->show();
    ui->pushButton_3->show();
    ui->pushButton_4->show();*/
}

void MainWindow::onUpdateTime()
{
    m_currentTimeLabel->setText(QDateTime::currentDateTime().toString(" yyyy年M月d日 hh:mm:ss "));//设置显示的格式

    if(!m_runningTask.isEmpty())
    {
        if(!m_cmd->isOpen())
        {
            m_cmd->start(QString("./rsyncclient -D -L6 -f %1").arg(g_MainMod->getTask(*m_runningTask.begin())->getXmlPath()));
            g_MainMod->setTaskStatus(*m_runningTask.begin(), Task::TaskStatus::Running);
            ui->progressBar->show();
            ui->pushButton_3->show();
            ui->pushButton_4->show();
            m_runningTask.erase(m_runningTask.begin());
        }
    }
}

void MainWindow::onCmdReadOutput()
{
    while (m_cmd->canReadLine())
    {
        auto str = m_cmd->readLine();
        ui->textBrowser->append(LogInterpreter::praseLoggerForTextBrower(str));
    }
    //ui->textBrowser->setText(m_cmdoutput); //频繁更新会很卡

    /*static QDateTime last(QDateTime::currentDateTime());
    if(last.msecsTo(QDateTime::currentDateTime()) > 1000)
    {
        ui->textBrowser->setText(m_cmdoutput);
    }*/
}

void MainWindow::on_pushButton_3_clicked()  //终止当前执行的任务
{
    if (m_cmd->isOpen())
    {
        m_cmd->kill();
    }
}

void MainWindow::onCmdFinished()
{
    ui->progressBar->hide();
    ui->pushButton_3->hide();
    ui->pushButton_4->hide();
    m_cmdoutput.clear();
    m_cmd->close();
}

void MainWindow::showStatusBarTip(const QString &tip)
{
    ui->statusbar->showMessage(tip);
    QApplication::processEvents();
}

void MainWindow::addTask(const Task &task)
{
    QString indexStr = QDateTime::fromMSecsSinceEpoch(task.m_id).toString(QStringLiteral("yyyy年MM月dd日"));
    auto l = m_taskDateModel->findItems(indexStr);

    if(l.isEmpty())
    {
        m_taskDateModel->appendRow(new QStandardItem(indexStr));
        auto tl = new QStandardItemModel(this);
        tl->setHorizontalHeaderLabels(
                QStringList() << QStringLiteral("id") << QStringLiteral("创建时间") << QStringLiteral("状态")
                              << QStringLiteral("类型") << QStringLiteral("本地路径") << QStringLiteral("远程路径"));
        m_taskList[indexStr] = tl;
    }

    auto nT = new QStandardItem(QString::number(task.m_id));
    nT->setCheckable(true);
    m_taskList[indexStr]->appendRow(nT);

    auto root = m_taskList[indexStr]->invisibleRootItem();
    root->setChild(nT->index().row(), 1, new QStandardItem(QDateTime::fromMSecsSinceEpoch(task.m_id).toString("yyyy-MM-dd hh:mm:ss")));
    root->setChild(nT->index().row(), 2, new QStandardItem(QString::number(task.m_status)));
    root->setChild(nT->index().row(), 3, new QStandardItem(QString::number(task.m_type)));
    root->setChild(nT->index().row(), 4, new QStandardItem(task.m_src.first() + "..."));
    root->setChild(nT->index().row(), 5, new QStandardItem(task.m_des.first() + "..."));

}

void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    ui->treeView_2->setModel(m_taskList[m_taskDateModel->itemFromIndex(index)->text()]);
    ui->treeView_2->setColumnWidth(0, 150);
    ui->treeView_2->setColumnWidth(1, 150);
    ui->treeView_2->setColumnWidth(2, 150);
    ui->treeView_2->setColumnWidth(3, 150);
    ui->treeView_2->setColumnWidth(4, 200);
    ui->treeView_2->setColumnWidth(5, 200);
}

void MainWindow::on_treeView_2_doubleClicked(const QModelIndex &index)
{
    auto id = dynamic_cast<const QStandardItemModel*>(index.model())->itemFromIndex(index)->text();

    const Task* pTask = g_MainMod->getTask(id.toLong());

    if(pTask != nullptr)
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(pTask->getXmlPath()));
    }

}


void MainWindow::on_pushButton_2_clicked()
{
    /*auto index = ui->treeView_2->currentIndex();
    auto id = dynamic_cast<const QStandardItemModel*>(index.model())->itemFromIndex(index)->text().toLong();

    const Task* pTask = g_MainMod->getTask(id);
    if(pTask != nullptr)
    {
        m_runningTask.insert(pTask->m_id);
        g_MainMod->setTaskStatus(pTask->m_id, Task::TaskStatus::Waiting);
    }*/

    auto parent = dynamic_cast<const QStandardItemModel*>(ui->treeView_2->model())->invisibleRootItem();

    int count = parent->rowCount();
    for(int i = 0; i < count ; i++)
    {
        QStandardItem* pItem = parent->child(i);
        if(pItem)
        {
            if(pItem->data(Qt::CheckStateRole) == Qt::Checked)
            {
                const Task* pTask = g_MainMod->getTask(pItem->text().toLong());
                if(pTask)
                {
                    m_runningTask.insert(pTask->m_id);
                    g_MainMod->setTaskStatus(pTask->m_id, Task::TaskStatus::Waiting);
                    parent->child(pItem->index().row(), 2)->setText(QString::number(pTask->m_status));
                    pItem->setData(Qt::Unchecked, Qt::CheckStateRole);
                }
            }
        }
    }
}
