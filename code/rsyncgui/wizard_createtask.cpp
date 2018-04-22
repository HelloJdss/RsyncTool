#include <QFileDialog>
#include <QDebug>
#include <QDateTime>
#include <QProcess>
#include <QtXml/QDomDocument>
#include <QtCore/QXmlStreamReader>


#include "wizard_createtask.h"
#include "ui_wizard_createtask.h"

Wizard_CreateTask::Wizard_CreateTask(QWidget *parent) :
        QWizard(parent),
        ui(new Ui::Wizard_CreateTask)
{
    ui->setupUi(this);

    //m_srcModel = new QFileSystemModel_CheckBox();
    m_srcModel = new myDirModel(this);
    ui->treeView->setModel(m_srcModel);
    ui->treeView->setColumnWidth(0, 275);

    //ui->lineEdit->setText(QDir::currentPath() + QDir::separator());

    m_desModel = new myRemoteDirModel(this);
    ui->treeView_2->setModel(m_desModel->getModelPtr());

    m_logInterpreter = new LogInterpreter();
    /*connect(m_logInterpreter, &LogInterpreter::onFindViewDir, [=](QString path, int64_t size, int64_t modify)
    {
        m_desModel->appendInfo(path, size, modify);
    });*/

    m_cmd = new QProcess(this);
    connect(m_cmd, &QProcess::readyRead, [=]()
    {
        while (m_cmd->canReadLine())
        {
            g_MainMod->showStatusTip(QStringLiteral("执行：") + m_cmd->readLine());
        }
    });

    connect(m_cmd, static_cast<void (QProcess::*)(int)>(&QProcess::finished), this,
            &Wizard_CreateTask::onViewDirFinished);

}

Wizard_CreateTask::~Wizard_CreateTask()
{
    delete m_logInterpreter;
    delete ui;
}

void Wizard_CreateTask::on_pushButton_clicked()
{
    //只允许选择文件夹
    QString dirName = QFileDialog::getExistingDirectory(this, QStringLiteral("目录选择"), ".");

    if (dirName.isEmpty())
    {
        return;
    }

    ui->lineEdit->setText(dirName + QDir::separator());
    on_lineEdit_editingFinished();
}

void Wizard_CreateTask::on_Wizard_CreateTask_currentIdChanged(int id)
{
    if (ui->radioButton->isChecked())
    {
        //本地到服务器，显示复选框
        ui->treeView->show();
        ui->label_7->show();
        ui->line_5->show();

        ui->treeView_2->hide();
        ui->label_9->hide();
        ui->line_9->hide();
        ui->pushButton_2->hide();

        ui->label_14->setText(ui->radioButton->text());

        if (!ui->lineEdit_3->text().endsWith(QDir::separator()))
        {
            ui->lineEdit_3->setText(ui->lineEdit_3->text().append(QDir::separator()));
        }
    }
    else
    {
        ui->treeView->hide();
        ui->label_7->hide();
        ui->line_5->hide();

        ui->treeView_2->show();
        ui->label_9->show();
        ui->line_9->show();
        ui->pushButton_2->show();

        ui->label_14->setText(ui->radioButton_2->text());

        if (!ui->lineEdit->text().endsWith(QDir::separator()))
        {
            ui->lineEdit->setText(ui->lineEdit->text().append(QDir::separator()));
        }
    }

    if (id == pageIds().last())
    {
        //如果是最后一页

        ui->label_15->setText(ui->lineEdit_2->text() + ":" + ui->spinBox->text());

        ui->textBrowser->clear();
        ui->textBrowser_2->clear();


        if (ui->radioButton->isChecked())
        {
            auto src = m_srcModel->getCheckedInfo();
            for (const auto &item : src)
            {
                ui->textBrowser->append(item);
            }

            ui->textBrowser_2->append(ui->lineEdit_3->text());
        }
        else
        {
            ui->textBrowser->append(ui->lineEdit->text());

            auto des = m_desModel->getCheckedInfo();
            for (const auto &item : des)
            {
                ui->textBrowser_2->append(item);
            }
        }
    }
}

Task Wizard_CreateTask::GetTask()
{
    /*
     * 生成任务信息
     */

    if (ui->radioButton->isChecked())
    {
        return Task(Task::PUSH, m_srcModel->getCheckedInfo(), QStringList() << ui->lineEdit_3->text(),
                    ui->lineEdit_2->text(),
                    static_cast<uint16_t>(ui->spinBox->value()));
    }

    if (ui->radioButton_2->isChecked())
    {
        return Task(Task::PULL, QStringList() << ui->lineEdit->text(), m_desModel->getCheckedInfo(),
                    ui->lineEdit_2->text(),
                    static_cast<uint16_t>(ui->spinBox->value()));
    }

}

void Wizard_CreateTask::on_lineEdit_4_editingFinished() //文件过滤信息
{
    m_srcModel->setNameFilters(ui->lineEdit_4->text().split(";"));

    m_srcModel->clearCheckedIndexes();
    m_srcModel->index(ui->lineEdit->text());

    ui->treeView->update();

    m_desModel->setFilter(ui->lineEdit_4->text().split(";"));

    ui->treeView_2->update();
}

void Wizard_CreateTask::on_lineEdit_editingFinished() //src编辑完毕
{
    m_srcModel->clearCheckedIndexes();

    if (ui->radioButton->isChecked())
    {
        auto index = m_srcModel->index(ui->lineEdit->text());
        m_srcModel->setData(index, Qt::Checked, Qt::CheckStateRole);
        ui->treeView->setRootIndex(index);
    }
}

void Wizard_CreateTask::on_lineEdit_returnPressed()
{
    on_lineEdit_editingFinished();
}

void Wizard_CreateTask::on_pushButton_2_clicked() //des编辑完毕
{
    m_desModel->setRootDir(ui->lineEdit_3->text(), ui->lineEdit_4->text().split(";"));

    m_tmpName = QDir::tempPath() + QDir::separator() + QApplication::applicationName() +
                QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") + ".xml";
    QString cmd = QString("./rsyncclient -D -v %1@%2:%3 -o %4").arg(ui->lineEdit_3->text()).arg(ui->lineEdit_2->text())
            .arg(ui->spinBox->value()).arg(m_tmpName);

    qDebug() << cmd;

    m_progress_run = true;
    m_cmd->setWorkingDirectory(QDir::currentPath());
    m_cmd->start(cmd);



    //m_cmd->waitForFinished(-1);

    if (m_progress)
    {
        delete m_progress;
    }
    m_progress = new QProgressDialog(this);
    m_progress->setLabelText(QStringLiteral("正在获取文件列表"));
    m_progress->setWindowTitle(QStringLiteral("执行任务中"));
    m_progress->setRange(0, 0);
    m_progress->setWindowModality(Qt::WindowModal);
    m_progress->show();
    //dialog.setCancelButton(nullptr);

    while (m_progress_run)
    {
        QApplication::processEvents();
        if (m_progress->wasCanceled())
        {
            m_cmd->kill();
            m_progress_run = false;
        }
    }
    //QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("连接超时"));

    //ui->treeView_2->update();
}

void Wizard_CreateTask::onViewDirFinished()
{
    ui->treeView_2->setColumnWidth(0, 250);

    QFile file(m_tmpName);
    QDomDocument document;

    //if (!file.open(QIODevice::ReadOnly) || !document.setContent(&file))
    if (!file.open(QIODevice::ReadOnly))
    {
        file.close();
        file.remove();
        QMessageBox::critical(this, QStringLiteral("错误！"), QStringLiteral("获取远程目录失败"));
        return;
    }

    if (m_progress)
    {
        m_progress->setLabelText(QStringLiteral("正在重建文件列表"));
    }

    QXmlStreamReader reader;

    reader.setDevice(&file);

    while (!reader.atEnd())
    {
        if (reader.readNext() == QXmlStreamReader::StartElement)
        {
            if (reader.name().toString() == "File")
            {
                auto path = reader.attributes().value("Path").toString();
                auto size = reader.attributes().value("Size").toLong();
                auto modify = reader.attributes().value("Modify").toLong();

                if (!path.isEmpty())
                {
                    m_desModel->appendInfo(path, size, modify);

                    g_MainMod->showStatusTip(QStringLiteral("扫描：") + path);
                }
            }
        }

        if (reader.hasError())
        {
            qDebug() << "error " << reader.errorString();
        }

        if (!m_progress || m_progress->wasCanceled())
        {
            break;
        }



        QApplication::processEvents();
    }

    file.close();
    //file.remove();

    m_progress_run = false;
    m_progress->close();

    /*QDomElement domElement = document.documentElement();

    QDomNodeList list = document.elementsByTagName("File");

    QProgressDialog dialog(this);
    dialog.setLabelText(QStringLiteral("正在重建文件列表视图"));
    dialog.setWindowTitle(QStringLiteral("执行任务中"));
    dialog.setRange(0, list.count());
    dialog.setWindowModality(Qt::WindowModal);
    dialog.show();

    for (int i = 0; i < list.count(); ++i)
    {
        QDomElement e = list.at(i).toElement();
        auto path = e.attribute("Path");
        auto size = e.attribute("Size").toLong();
        auto modify = e.attribute("Modify").toLong();

        if (!path.isEmpty())
        {
            m_desModel->appendInfo(path, size, modify);
        }

        dialog.setValue(i + 1);

        QApplication::processEvents();
        if(dialog.wasCanceled())
        {
            break;
        }
    }*/

    ui->treeView_2->update();
}

