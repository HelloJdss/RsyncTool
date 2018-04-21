//
// Created by carrot on 18-4-20.
//

#ifndef RSYNCTOOL_MYMODEL_H
#define RSYNCTOOL_MYMODEL_H

#include <QSet>
#include <QDirModel>
#include <QStandardItemModel>
#include <QMap>

/**
 * 一个带选择框的QDirModel，用于本地文件监控
 */

class myDirModel : public QDirModel
{
public:
    explicit myDirModel(QObject *parent = Q_NULLPTR);

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

public:
    void clearCheckedIndexes();

    QStringList getCheckedInfo();

private:
    void recursiveCheck(const QModelIndex &index, const QVariant &value);

    QSet<QPersistentModelIndex> m_checkedIndexes;

    QStringList m_header;
};

/**
 * 利用QStandardItemModel对远程文件的整理
 */
class myRemoteDirModel : public QObject
{
    Q_OBJECT

public:
    myRemoteDirModel(QObject *parent = Q_NULLPTR);

    ~myRemoteDirModel();

    void setRootDir(QString root);

    void appendInfo(QString path, int64_t size, int64_t modify);

    QStandardItemModel *getModelPtr() const
    { return m_model; }

    QStringList getCheckedInfo();

private:
    void reset();

    void onItemChanged(QStandardItem *item);

    void checkAllChild(QStandardItem *item, bool check);

    void CheckChildChanged(QStandardItem *item);

    Qt::CheckState checkSibling(QStandardItem *item);

    QMap<QString, QStandardItem*> m_prefix;

    QSet<QStandardItem*> m_checkedItem;

    QMap<QStandardItem*, QString> m_path;   //完整路径

    QStandardItemModel *m_model = nullptr;

    QString m_root;
};

#endif //RSYNCTOOL_MYMODEL_H
