//
// Created by carrot on 18-4-19.
//

#include "MainMod.h"
#include <QFile>
#include <QDateTime>

Task::Task(Task::TaskType type, const QStringList &src, const QStringList &des, const QString &ip, uint16_t port)
{
    m_type = type;
    m_src = src;
    m_des = des;
    m_desIP = ip;
    m_desPort = port;
    m_createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

Task::Task()
{
    m_des.clear();
    m_src.clear();
}

QString MainMod::GetLastErr()
{
    return m_lastErr;
}

bool MainMod::LoadConfig(QSplashScreen *screen)
{
    //检查命令行程序是否存在
    if(screen)
    {
        screen->showMessage(QStringLiteral("Checking command-line interface modules..."), Qt::AlignLeft | Qt::AlignBottom);
    }

    if(!QFile::exists("rsyncclient"))
    {
        m_lastErr = "can not find \'rsyncclient\'!";
        return false;
    }


    if(screen)
    {
        screen->showMessage(QStringLiteral("Check complete!"), Qt::AlignLeft | Qt::AlignBottom);
    }
    return true;
}

bool MainMod::addTask(Task const &task)
{
    //TODO: 程序是否需要考虑添加的各个任务直接执行是否会发生死锁？（例如一边推送，一边从同一个目录接收，或者同步了日志目录，导致当天的日志不断地产生，不断的生成签名信息）
    m_tasks.push_back(task);

    return true;
}


