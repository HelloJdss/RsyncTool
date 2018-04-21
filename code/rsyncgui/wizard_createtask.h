#ifndef WIZARD_CREATETASK_H
#define WIZARD_CREATETASK_H

#include <QWizard>
#include "MainMod.h"
#include "myModel.h"
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>

namespace Ui
{
    class Wizard_CreateTask;
}

/**
 * http://tcspecial.iteye.com/blog/1880711
 * 实现浏览文件复选模型
 */

class Wizard_CreateTask : public QWizard
{
Q_OBJECT

public:
    explicit Wizard_CreateTask(QWidget *parent = 0);

    ~Wizard_CreateTask() override;

    Task GetTask(); //执行结束后调用此函数获取任务

private slots:

    void on_pushButton_clicked();

    void on_Wizard_CreateTask_currentIdChanged(int id);

    void on_lineEdit_4_editingFinished();

    void on_lineEdit_editingFinished();

    void on_lineEdit_returnPressed();

    void on_pushButton_2_clicked();

private:

    void onViewDirFinished();

    Ui::Wizard_CreateTask *ui;
    //QFileSystemModel_CheckBox *m_srcModel = nullptr;
    myDirModel *m_srcModel = nullptr;
    myRemoteDirModel *m_desModel = nullptr;
    LogInterpreter *m_logInterpreter = nullptr;
    QProcess *m_cmd = nullptr;
    QString m_tmpName;
    //QProgressDialog *m_progress = nullptr;
    volatile bool  m_progress_run = false;
};

#endif // WIZARD_CREATETASK_H
