//
// Created by carrot on 18-4-19.
//

#ifndef RSYNCTOOL_MAINMOD_H
#define RSYNCTOOL_MAINMOD_H

#include "cm_define.h"
#include <QSplashScreen>
#include <QVector>

class Task  //定义一个任务详细信息
{
public:
    enum TaskType
    {
        PUSH, PULL
    };
    Task();

    Task(TaskType type, const QStringList& src, const QStringList& des, const QString& ip, uint16_t port);

public:
    QString m_createTime;
    TaskType m_type;
    QStringList m_src;
    QStringList m_des;
    QString m_desIP;
    uint16_t m_desPort;
};

class MainMod
{
DECLARE_SINGLETON_EX(MainMod)

public:
    bool LoadConfig(QSplashScreen *screen = nullptr); //预加载配置，如果有启动画面，则输出信息

    QString GetLastErr();

    bool addTask(const Task &task);

private:
    QString m_lastErr;   //最后一次的错误
    QVector<Task> m_tasks;     //任务列表
};

#define g_MainMod MainMod::Instance()

#endif //RSYNCTOOL_MAINMOD_H