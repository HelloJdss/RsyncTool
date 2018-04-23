//
// Created by carrot on 18-4-19.
//

#include <QDateTime>
#include <QRegularExpression>
#include <cm_logger.h>
#include "MainMod.h"
#include "mainwindow.h"
#include <QFile>
#include <QDir>
#include <QXmlStreamWriter>

Task::Task(Task::TaskType type, const QStringList &src, const QStringList &des, const QString &ip, uint16_t port)
{
    //m_createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    m_id = QDateTime::currentMSecsSinceEpoch();
    m_type = type;
    m_src = src;
    m_des = des;
    m_desIP = ip;
    m_desPort = port;
    m_status = Ready;
}

Task::Task()
{
    m_id = QDateTime::currentMSecsSinceEpoch();
    m_des.clear();
    m_src.clear();
    m_status = Ready;
}

bool Task::saveAsXml() const
{
    QFile file(QDir::currentPath() + QDir::separator() + "tasks" + QDir::separator() + QString::number(m_id) + ".xml");
    QDir::current().mkdir(QDir::currentPath() + QDir::separator() + "tasks");

    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug() << "error: open failed " + file.fileName();
        return false;
    }
    g_MainMod->showStatusTip(QStringLiteral("保存：") + file.fileName());

    QXmlStreamWriter streamWriter(&file);
    streamWriter.setAutoFormatting(true);
    streamWriter.writeStartDocument();

    streamWriter.writeStartElement("Task");
    streamWriter.writeAttribute("id", QString::number(m_id));
    streamWriter.writeAttribute("createTime", QDateTime::fromMSecsSinceEpoch(m_id).toString("yyyy-MM-dd hh:mm:ss"));
    streamWriter.writeAttribute("type", QString::number(m_type));
    streamWriter.writeAttribute("status", QString::number(m_status));

    streamWriter.writeStartElement("Src");
    for (const auto &item : m_src)
    {
        streamWriter.writeTextElement("Path", item);
    }
    streamWriter.writeEndElement();

    streamWriter.writeStartElement("Des");
    streamWriter.writeAttribute("ip", m_desIP);
    streamWriter.writeAttribute("port", QString::number(m_desPort));
    for (const auto &item : m_des)
    {
        streamWriter.writeTextElement("Path", item);
    }
    streamWriter.writeEndElement();

    streamWriter.writeEndElement();
    streamWriter.writeEndDocument();

    m_path = file.fileName();
    file.close();
    return true;
}

QString Task::getXmlPath() const
{
    if(m_path.isEmpty())
    {
        saveAsXml();
    }
    return m_path;
}

Task Task::loadFromXml(QString path)
{
    Task task;
    task.m_id = -1;

    QFile file(path);
    if(!file.open(QFile::ReadOnly | QFile::Text))
    {
        return task;
    }

    QXmlStreamReader reader;
    reader.setDevice(&file);

    QStringList parent;

    while (!reader.atEnd())
    {
        auto type = reader.readNext();
        if(type == QXmlStreamReader::StartElement)
        {
            //qDebug() << reader.name();
            parent << reader.name().toString();
            if(reader.name() == "Task")
            {
                task.m_id = reader.attributes().value("id").toLong();
                task.m_type = static_cast<TaskType>(reader.attributes().value("type").toInt());
                task.m_status = static_cast<TaskStatus>(reader.attributes().value("status").toInt());
            }
            else if(reader.name() == "Des")
            {
                task.m_desIP = reader.attributes().value("ip").toString();
                task.m_desPort = static_cast<uint16_t>(reader.attributes().value("port").toInt());
            }
            else if(reader.name() == "Path")
            {
                if(parent.contains("Des"))
                {
                    task.m_des << reader.readElementText();
                }
                else if (parent.contains("Src"))
                {
                    task.m_src << reader.readElementText();
                }
            }
        }
        else if(type == QXmlStreamReader::EndElement)
        {
            parent.removeOne(reader.name().toString());
        }
    }

    return task;
}

QString MainMod::GetLastErr()
{
    return m_lastErr;
}

bool MainMod::LoadDataOnStart(QSplashScreen *screen)
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

    QString tasksDirName = QDir::currentPath() + QDir::separator() + "tasks";

    QDir tasksDir(tasksDirName);
    if(tasksDir.exists())
    {
        tasksDir.setNameFilters(QStringList() << "*.xml");
        QStringList list = tasksDir.entryList();
        for (const auto& item : list)
        {
            if (screen)
            {
                screen->showMessage(QStringLiteral("Add task: ") + item , Qt::AlignLeft | Qt::AlignBottom);
            }

            Task task = Task::loadFromXml(tasksDir.path() + QDir::separator() + item);
            if(task.m_id != -1)
            {
                g_MainMod->addTask(task);
            }
        }
    }

    if (screen)
    {
        screen->showMessage(QStringLiteral("Check complete!"), Qt::AlignLeft | Qt::AlignBottom);
    }
    return true;
}

Task const & MainMod::addTask(Task const &task)
{
    //TODO: 程序是否需要考虑添加的各个任务直接执行是否会发生死锁？（例如一边推送，一边从同一个目录接收，或者同步了日志目录，导致当天的日志不断地产生，不断的生成签名信息）

    if(m_mainWindow)
    {
        m_mainWindow->addTask(task);
    }

    qDebug() << "Add Task :" << task.m_id;

    return m_tasks[task.m_id] = task;
}

void MainMod::showStatusTip(const QString &tip)
{
    if (m_mainWindow)
    {
        m_mainWindow->showStatusBarTip(tip);
    }
}

void MainMod::setMainWindow(MainWindow *mainWindow)
{
    m_mainWindow = mainWindow;
}

bool MainMod::SaveDataOnClose()
{
    for(const auto& item : m_tasks)
    {
        item.saveAsXml();
    }
    return true;
}

Task const *MainMod::getTask(qint64 id)
{
    if(m_tasks.contains(id))
    {
        return &m_tasks[id];
    }
    return nullptr;
}

void MainMod::setTaskStatus(qint64 id, Task::TaskStatus status)
{
    if(m_tasks.contains(id))
    {
        m_tasks[id].m_status = status;
    }
}


QIcon QFileInfoEx::getFileIcon(const QString &filename)
{
    QString extension = filename.mid(filename.lastIndexOf('.'));

    QFileIconProvider provider;
    QIcon icon;
    QString strTemplateName = QDir::tempPath() + QDir::separator() + QCoreApplication::applicationName() +
                              "_XXXXXX" + extension;
    QTemporaryFile tmpFile(strTemplateName);
    //tmpFile.setAutoRemove(false);

    if (tmpFile.open())
    {
        tmpFile.close();
        icon = provider.icon(QFileInfo(tmpFile.fileName()));
        //tmpFile.remove();
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
    //tmpFile.setAutoRemove(false);

    if (tmpFile.open())
    {
        tmpFile.close();
        strType = provider.type(QFileInfo(tmpFile.fileName()));
        //tmpFile.remove();
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
    for (int i = 0; i < NR_LOGGERS; i++)
    {
        //qDebug() << g_loggerdef[i].pattern;
        re.setPattern(g_loggerdef[i].pattern);
        match = re.match(logger);
        if (match.hasMatch())
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
    for (int i = 0; i < NR_LOGGERS; i++)
    {
        //qDebug() << g_loggerdef[i].pattern;
        re.setPattern(g_loggerdef[i].pattern);
        match = re.match(logger);
        if (match.hasMatch())
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

QString LogInterpreter::praseLoggerForStatusTip(QString logger)
{
    logger.remove("\33[1;35m");
    logger.remove("\33[1;31m");
    logger.remove("\33[1;33m");
    logger.remove("\33[1;34m");
    logger.remove("\33[1;32m");
    logger.remove("\33[0m");
    return logger;
}
