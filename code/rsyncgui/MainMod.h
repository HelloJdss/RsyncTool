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

    enum TaskStatus
    {
        Ready, Waiting, Running, Abort, Finished,
    };
    Task();

    Task(TaskType type, const QStringList& src, const QStringList& des, const QString& ip, uint16_t port);

    bool saveAsXml() const;

    static Task loadFromXml(QString path);

    QString getXmlPath() const;

public:
    qint64 m_id;
    //QString m_createTime;
    TaskType m_type;
    TaskStatus m_status;
    QStringList m_src;
    QStringList m_des;
    QString m_desIP;
    uint16_t m_desPort;

private:
    mutable QString m_path;
};

class MainMod
{
DECLARE_SINGLETON_EX(MainMod)

public:
    bool LoadDataOnStart(QSplashScreen *screen = nullptr); //预加载数据，如果有启动画面，则输出信息

    bool SaveDataOnClose(); //离开时做保存和清理工作

    QString GetLastErr();

    Task const & addTask(const Task &task);

    Task const * getTask(qint64 id);

    void setTaskStatus(qint64 id, Task::TaskStatus status);

    void setMainWindow(MainWindow* mainWindow);

    void showStatusTip(const QString& tip);
private:
    QString m_lastErr;   //最后一次的错误
    QMap<qint64 ,Task> m_tasks;     //任务列表

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

    static QString praseLoggerForStatusTip(QString logger);

    static LoggerType getLoggerType(QString logger);

signals:
    void onFindViewDir(QString, int64_t, int64_t);


private:
    QString pretreatment(QString logger); //日志预处理

    void organize(QString logger); //整理日志

    QStringList m_logger;   //全部的日志内容


};

#endif //RSYNCTOOL_MAINMOD_H