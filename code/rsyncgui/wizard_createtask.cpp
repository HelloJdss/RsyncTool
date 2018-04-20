#include <QFileDialog>
#include <QDebug>

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

    ui->lineEdit->setText(QDir::currentPath() + "/");
    on_lineEdit_editingFinished();
}

Wizard_CreateTask::~Wizard_CreateTask()
{
    //delete m_srcModel;
    delete ui;
}

void Wizard_CreateTask::on_pushButton_clicked()
{
    //只允许选择文件夹
    QString dirName = QFileDialog::getExistingDirectory(this, QStringLiteral("目录选择"), ".");

    if(dirName.isEmpty())
    {
        return;
    }

    ui->lineEdit->setText(dirName + "/");
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
    qDebug() << m_srcModel->getCheckedPathes();
    return Task(ui->radioButton->isChecked() ? Task::PUSH : Task::PULL, QStringList() << ui->lineEdit->text(),
                QStringList() << ui->lineEdit_3->text(), ui->lineEdit_2->text(),
                static_cast<uint16_t>(ui->spinBox->value()));
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
