#include <QFileDialog>
#include <QDebug>
#include <QDateTime>
#include <QProcess>
#include <QtXml/QDomDocument>


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

    ui->lineEdit->setText(QDir::currentPath() + QDir::separator());
    on_lineEdit_editingFinished();

    m_desModel = new myRemoteDirModel(this);
    ui->treeView_2->setModel(m_desModel->getModelPtr());
    ui->treeView_2->setColumnWidth(0, 250);

    m_logInterpreter = new LogInterpreter();
    /*connect(m_logInterpreter, &LogInterpreter::onFindViewDir, [=](QString path, int64_t size, int64_t modify)
    {
        m_desModel->appendInfo(path, size, modify);
    });*/

    m_cmd = new QProcess(this);
    /*connect(m_cmd, &QProcess::readyRead, [=]()
    {
        while (m_cmd->canReadLine())
        {
            m_logInterpreter->addLine(m_cmd->readLine());
        }
    });*/

    connect(m_cmd, static_cast<void (QProcess::*)(int)>(&QProcess::finished),  this, &Wizard_CreateTask::onViewDirFinished);

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

void Wizard_CreateTask::on_lineEdit_4_editingFinished()
{
    m_srcModel->setNameFilters(ui->lineEdit_4->text().split(";"));

    m_srcModel->clearCheckedIndexes();
    m_srcModel->index(ui->lineEdit->text());

    ui->treeView->update();
}

void Wizard_CreateTask::on_lineEdit_editingFinished()
{
    if (ui->radioButton->isChecked())
    {
        m_srcModel->clearCheckedIndexes();
        auto index = m_srcModel->index(ui->lineEdit->text());
        m_srcModel->setData(index, Qt::Checked, Qt::CheckStateRole);
        ui->treeView->setRootIndex(index);
    }
}

void Wizard_CreateTask::on_lineEdit_returnPressed()
{
    on_lineEdit_editingFinished();
}

void Wizard_CreateTask::on_pushButton_2_clicked()
{
    m_desModel->setRootDir(ui->lineEdit_3->text());

    m_tmpName = QDateTime::currentDateTime().toString("yyyy_MM_dd_hh_mm_ss") + ".xml";
    QString cmd = QString("./rsyncclient -D -v %1@%2:%3 -o %4").arg(ui->lineEdit_3->text()).arg(ui->lineEdit_2->text())
            .arg(ui->spinBox->value()).arg(m_tmpName);

    qDebug() << cmd;

    m_progress_run = true;
    m_cmd->setWorkingDirectory(QDir::currentPath());
    m_cmd->start(cmd);



    //m_cmd->waitForFinished(-1);

    QProgressDialog dialog(this);
    dialog.setLabelText(QStringLiteral("正在获取文件列表"));
    dialog.setWindowTitle(QStringLiteral("执行任务中"));
    dialog.setMinimum(0);
    dialog.setMaximum(0);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.show();
    //dialog.setCancelButton(nullptr);

    while (m_progress_run)
    {
        QCoreApplication::processEvents();
        if (dialog.wasCanceled())
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
    m_progress_run = false;

    QFile file(m_tmpName);
    QDomDocument document;

    if(!file.open(QIODevice::ReadOnly) || !document.setContent(&file))
    {
        file.close();
        file.remove();
        QMessageBox::critical(this, QStringLiteral("错误！"), QStringLiteral("获取远程目录失败"));
        return;
    }

    file.close();
    file.remove();

    QDomElement domElement = document.documentElement();

    QDomNodeList list = document.elementsByTagName("File");

    for(int i = 0; i < list.count(); ++i)
    {
        QDomElement e = list.at(i).toElement();
        auto path = e.attribute("Path");
        auto size = e.attribute("Size").toLong();
        auto modify = e.attribute("Modify").toLong();

        if(!path.isEmpty())
        {
            m_desModel->appendInfo(path, size, modify);
        }
    }

    ui->treeView_2->update();
}

