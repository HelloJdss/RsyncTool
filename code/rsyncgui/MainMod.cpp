//
// Created by carrot on 18-4-19.
//

#include <QFile>
#include <QDateTime>
#include <QtCore/QRegularExpression>
#include <cm_logger.h>
#include "MainMod.h"

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
    if (screen)
    {
        screen->showMessage(QStringLiteral("Checking command-line interface modules..."),
                            Qt::AlignLeft | Qt::AlignBottom);
    }

    if (!QFile::exists("rsyncclient"))
    {
        m_lastErr = "can not find \'rsyncclient\'!";
        return false;
    }


    if (screen)
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


QIcon QFileInfoEx::getFileIcon(const QString &filename)
{
    QString extension = filename.mid(filename.lastIndexOf('.'));

    QFileIconProvider provider;
    QIcon icon;
    QString strTemplateName = QDir::tempPath() + QDir::separator() + QCoreApplication::applicationName() +
                              "_XXXXXX" + extension;
    QTemporaryFile tmpFile(strTemplateName);
    tmpFile.setAutoRemove(false);

    if (tmpFile.open())
    {
        tmpFile.close();
        icon = provider.icon(QFileInfo(tmpFile.fileName()));
        // tmpFile.remove();
    }
    else
    {
        qCritical() << QString("failed to write temporary file %1").arg(tmpFile.fileName());
    }

    return icon;
}

QString QFileInfoEx::getFileType(const QString &filename)
{
    QString extension = filename.mid(filename.lastIndexOf('.'));

    QFileIconProvider provider;
    QString strType;
    QString strFileName = QDir::tempPath() + QDir::separator() + QCoreApplication::applicationName() +
                          "_XXXXXX" + extension;
    QTemporaryFile tmpFile(strFileName);
    tmpFile.setAutoRemove(false);

    if (tmpFile.open())
    {
        tmpFile.close();
        strType = provider.type(QFileInfo(tmpFile.fileName()));
        // tmpFile.remove();
    }
    else
    {
        qCritical() << QString("failed to write temporary file %1").arg(tmpFile.fileName());
    }

    return strType;
}

QString LogInterpreter::pretreatment(QString src)
{
    src.remove("\33[1;35m");
    src.remove("\33[1;31m");
    src.remove("\33[1;33m");
    src.remove("\33[1;34m");
    src.remove("\33[1;32m");
    src.remove("\33[0m");
    return src;
}

QString LogInterpreter::praseLoggerForTextBrower(QString str)
{
    str.replace("&", "&amp;");
    str.replace(">", "&gt;");
    str.replace("<", "&lt;");
    str.replace("\"", "&quot;");
    str.replace("\'", "&#39;");
    str.replace(" ", "&nbsp;");
    str.replace("\n", "<br>");
    str.replace("\r", "<br>");

    str.replace("\33[1;35m", "<span style=\" color:#8B008B;\">"); //FATAL
    str.replace("\33[1;31m", "<span style=\" color:#FF0000;\">"); //ERROR
    str.replace("\33[1;33m", "<span style=\" color:#FFD700;\">"); //WARN
    str.replace("\33[1;34m", "<span style=\" color:#4169E1;\">"); //DEBUG
    str.replace("\33[1;32m", "<span style=\" color:#00FF00;\">"); //TRACE
    str.replace("\33[0m", "</span>");
    return str;
}

void LogInterpreter::addLine(QString logger)
{
    auto str = pretreatment(logger);

    m_logger << str;

    organize(logger);
}

LoggerType LogInterpreter::getLoggerType(QString logger)
{
    QRegularExpression re;
    QRegularExpressionMatch match;
    for(int i = 0; i < NR_LOGGERS; i++)
    {
        //qDebug() << g_loggerdef[i].pattern;
        re.setPattern(g_loggerdef[i].pattern);
        match = re.match(logger);
        if(match.hasMatch())
        {
            qDebug() << "matched : " << match.captured(0);
            return g_loggerdef[i].type;
        }
    }
    return UnKnown;
}

void LogInterpreter::organize(QString logger)
{
    QRegularExpression re;
    QRegularExpressionMatch match;
    LoggerType type = UnKnown;
    for(int i = 0; i < NR_LOGGERS; i++)
    {
        //qDebug() << g_loggerdef[i].pattern;
        re.setPattern(g_loggerdef[i].pattern);
        match = re.match(logger);
        if(match.hasMatch())
        {
            //qDebug() << "matched : " << match.captured(0);
            type = g_loggerdef[i].type;
        }
    }

    switch (type)
    {
        case ViewDir:
            emit onFindViewDir(match.captured(1), match.captured(2).toLong(), match.captured(3).toLong());
        default:
            break;
    }
}
