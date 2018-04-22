//
// Created by carrot on 18-4-19.
//

#ifndef RSYNCTOOL_MAINMOD_H
#define RSYNCTOOL_MAINMOD_H

#include <QSplashScreen>
#include <QVector>
#include <QFileIconProvider>
#include <QDir>
#include <QTemporaryFile>
#include <QCoreApplication>
#include <QDebug>


#include "cm_define.h"
#include "cm_logger.h"

class MainWindow;

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

    void setMainWindow(MainWindow* mainWindow);

    void showStatusTip(const QString& tip);
private:
    QString m_lastErr;   //最后一次的错误
    QVector<Task> m_tasks;     //任务列表

    MainWindow* m_mainWindow = nullptr;
};

#define g_MainMod MainMod::Instance()

class QFileInfoEx
{
public:
/**
 * @brief 获取文件图标,可以针对本地不存在的文件
 * @param filename
 * @return 文件图标
 */
    static QIcon getFileIcon(const QString &filename);
/**
 * @brief 获取文件类型,可以针对本地不存在的文件
 * @param filename
 * @return 文件类型
 */
    static QString getFileType(const QString &filename);
};

/**
 * 对日志的解析器
 */
class LogInterpreter : public QObject
{
    Q_OBJECT

public:
    void addLine(QString logger);

    static QString praseLoggerForTextBrower(QString logger);

    static LoggerType getLoggerType(QString logger);

signals:
    void onFindViewDir(QString, int64_t, int64_t);


private:
    QString pretreatment(QString logger); //日志预处理

    void organize(QString logger); //整理日志

    QStringList m_logger;   //全部的日志内容


};

#endif //RSYNCTOOL_MAINMOD_H