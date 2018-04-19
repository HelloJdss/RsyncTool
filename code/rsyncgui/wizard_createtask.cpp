#include <QFileDialog>
#include <QDebug>

#include "wizard_createtask.h"
#include "ui_wizard_createtask.h"


Wizard_CreateTask::Wizard_CreateTask(QWidget *parent) :
        QWizard(parent),
        ui(new Ui::Wizard_CreateTask)
{
    ui->setupUi(this);

    m_srcModel = new QFileSystemModel(this);
}

Wizard_CreateTask::~Wizard_CreateTask()
{
    delete ui;
}

void Wizard_CreateTask::on_pushButton_clicked()
{
    //只允许选择文件夹
    QString dirName = QFileDialog::getExistingDirectory(this, QStringLiteral("目录选择"), ".");
    ui->lineEdit->setText(dirName + "/");

    if (ui->radioButton->isChecked())
    {
        m_srcModel->setRootPath(dirName);

        ui->treeView->setModel(m_srcModel);
        ui->treeView->setRootIndex(m_srcModel->index(dirName));
    }
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
    return Task(ui->radioButton->isChecked() ? Task::PUSH : Task::PULL, QStringList() << ui->lineEdit->text(),
                QStringList() << ui->lineEdit_3->text(), ui->lineEdit_2->text(),
                static_cast<uint16_t>(ui->spinBox->value()));
}
