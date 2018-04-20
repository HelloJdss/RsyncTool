//
// Created by carrot on 18-4-20.
//

#ifndef RSYNCTOOL_MYMODEL_H
#define RSYNCTOOL_MYMODEL_H

#include <QSet>
#include <QFileSystemModel>
#include <QDirModel>

/**
 * 一个带选择框的QFileSystemModel
 */

class myFileSystemModel : public QFileSystemModel
{
public:
    explicit myFileSystemModel(QObject *parent = Q_NULLPTR);

    Qt::ItemFlags flags(const QModelIndex &index) const ;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

private:
    mutable QMap<QModelIndex, bool> m_indexMap;

    QSet<QString> m_checkedFileList;
};

/**
 * 一个带选择框的QDirModel
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

    QStringList getCheckedPathes();

private:
    void recursiveCheck(const QModelIndex &index, const QVariant &value);

    QSet<QPersistentModelIndex> m_checkedIndexes;

    QStringList m_header;
};

#endif //RSYNCTOOL_MYMODEL_H
