//
// Created by carrot on 18-4-20.
//

#include "myModel.h"

#include <QDebug>

QVariant myFileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    qDebug() << index.column() << "," << index.row();

    if (index.column() == 0 && role == Qt::CheckStateRole)
    {
        if (m_indexMap.contains(index))
        {
            return m_indexMap[index] ? Qt::Checked : Qt::Unchecked;
        }
        else
        {
            int iChecked = Qt::Unchecked;
            QModelIndex _parentIndex = index.parent();

            if (_parentIndex.isValid())
            {
                if (m_indexMap[_parentIndex])
                {
                    iChecked = Qt::Checked;
                }
            }
            //check if this node's parent is checked
            /*while (_parentIndex.isValid())
            {
                if (m_indexMap[_parentIndex])
                {
                    iChecked = Qt::Checked;
                    break;
                }
                else
                {
                    _parentIndex = _parentIndex.parent();
                }
            }*/

            m_indexMap[index] = iChecked == Qt::Checked;

            return iChecked;
        }
    }

    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    return QFileSystemModel::data(index, role);
}

bool myFileSystemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        if (value == Qt::Unchecked)
        {
            m_indexMap[index] = false;
            //refresh it's child node
            emit dataChanged(index, index);
        }
        else if (value == Qt::Checked)
        {
            m_indexMap[index] = true;
            //refresh it's child node
            emit dataChanged(index, index);
        }

        if (hasChildren(index))
        {
            //QString strFilePath = filePath(index);
            //setFileCheckState(strFilePath, value);

            int iChildCount = rowCount(index);

            if (iChildCount > 0)
            {
                for (int i = 0; i < iChildCount; i++)
                {
                    QModelIndex _child = this->index(i, 0, index);
                    setData(_child, value, Qt::CheckStateRole);
                }
            }
        }
    }

    return QFileSystemModel::setData(index, value, role);
}

myFileSystemModel::myFileSystemModel(QObject *parent) : QFileSystemModel(parent)
{

}

Qt::ItemFlags myFileSystemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return 0;
    }

    if (index.column() == 0)
    {
        return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    return QFileSystemModel::flags(index);
}

myDirModel::myDirModel(QObject *parent) : QDirModel(parent)
{
    m_header << QStringLiteral("名称") << QStringLiteral("大小") << QStringLiteral("类型") << QStringLiteral("修改日期");
}

Qt::ItemFlags myDirModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return 0;
    }

    if (index.column() == 0)
    {
        return QDirModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    return QDirModel::flags(index);
}

QVariant myDirModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        return m_checkedIndexes.contains(index) ? Qt::Checked : Qt::Unchecked;
    }

    return QDirModel::data(index, role);
}

bool myDirModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
    {
        return false;
    }

    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        if (value == Qt::Checked)
        {
            m_checkedIndexes.insert(index);
            if (hasChildren(index))
            {
                recursiveCheck(index, value);
            }
        }
        else
        {
            m_checkedIndexes.remove(index);
            if (hasChildren(index))
            {
                recursiveCheck(index, value);
            }
        }
        emit dataChanged(index, index);
        return true;
    }

    return QDirModel::setData(index, value, role);
}

void myDirModel::recursiveCheck(const QModelIndex &index, const QVariant &value)
{
    if (!index.isValid())
    {
        return;
    }

    if (hasChildren(index))
    {
        int i;
        int childrenCount = rowCount(index);
        QModelIndex child;
        for (i = 0; i < childrenCount; i++)
        {
            child = QDirModel::index(i, 0, index);
            setData(child, value, Qt::CheckStateRole);
        }
    }
}

QVariant myDirModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal && section < m_header.length())
    {
        return m_header.at(section);
    }

    return QDirModel::headerData(section, orientation, role);
}

void myDirModel::clearCheckedIndexes()
{
    m_checkedIndexes.clear();
}

QStringList myDirModel::getCheckedPathes()
{
    QStringList ret;
    for(const auto& it : m_checkedIndexes)
    {
        if(QDirModel::isDir(it))
        {
            ret << QDirModel::filePath(it) + "/";
        }
        else
        {
            ret << QDirModel::filePath(it);
        }
    }
    return ret;
}
