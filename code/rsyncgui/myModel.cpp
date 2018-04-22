//
// Created by carrot on 18-4-20.
//

#include "myModel.h"
#include "MainMod.h"

#include <QDebug>
#include <QIcon>
#include <QDateTime>

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

QStringList myDirModel::getCheckedInfo()
{
    QStringList ret;
    for (const auto &it : m_checkedIndexes)
    {
        if (QDirModel::isDir(it))
        {
            ret << QDirModel::filePath(it) + QDir::separator();
        }
        else
        {
            ret << QDirModel::filePath(it);
        }
    }
    return ret;
}

myRemoteDirModel::myRemoteDirModel(QObject *parent) : QObject(parent)
{
    m_model = new QStandardItemModel(this);
    connect(m_model, &QStandardItemModel::itemChanged, this, &myRemoteDirModel::onItemChanged);

    reset();
}

void myRemoteDirModel::appendInfo(QString path, int64_t size, int64_t modify)
{
    int pos = -1;
    QString currentprefix;
    QFileIconProvider provider;
    QStandardItem *parent = m_model->invisibleRootItem();
    QString parentprefix;
    while (-1 != (pos = path.indexOf(QDir::separator(), pos + 1)))
    {
        currentprefix = path.left(pos + 1);

        //qDebug() << "prefix: " << currentprefix;

        if(m_root.startsWith(currentprefix))
        {
            parentprefix = currentprefix;
            continue;
        }


        if (!m_prefix.contains(currentprefix)) //不存在本级目录则创建本级目录
        {
            //qDebug() << "create !";
            auto item = new QStandardItem(provider.icon(provider.Folder),
                                          QString(currentprefix).remove(parentprefix).remove(QDir::separator()));
            item->setCheckable(true);
            item->setEditable(false);
            item->setData(Qt::Checked, Qt::CheckStateRole);
            m_checkedItem.insert(item);

            parent->appendRow(item);
            //parent->setChild(item->index().row(), 1, new QStandardItem(QString::number(size)));
            parent->setChild(item->index().row(), 1, new QStandardItem("-"));
            parent->setChild(item->index().row(), 2, new QStandardItem(QStringLiteral("文件夹")));
            parent->setChild(item->index().row(), 3, new QStandardItem("-"));
            m_prefix[currentprefix] = item;
        }

        parent = m_prefix[currentprefix];
        parentprefix = currentprefix;

    }

    auto name = QString(path).remove(parentprefix);
    //qDebug() << "name: " << name;

    if (!name.isEmpty())
    {
        auto item = new QStandardItem(QFileInfoEx::getFileIcon(name), name);
        item->setCheckable(true);
        item->setEditable(false);

        if(matchFilter(name))
        {
            item->setData(Qt::Checked, Qt::CheckStateRole);
            m_checkedItem.insert(item);
        }
        else
        {
            item->setEnabled(false);
        }

        parent->appendRow(item);
        parent->setChild(item->index().row(), 1, new QStandardItem(QString::number(size)));
        parent->setChild(item->index().row(), 2, new QStandardItem(QFileInfoEx::getFileType(name)));
        parent->setChild(item->index().row(), 3,
                         new QStandardItem(QDateTime::fromMSecsSinceEpoch(modify).toString("yyyy-MM-dd hh:mm:ss")));

        m_path[item] = path;
    }
    else
    {
        //说明是文件夹
        //parent->parent()->setChild(parent->index().row(), 3, new QStandardItem("-"));
                //QDateTime::fromMSecsSinceEpoch(modify).toString("yyyy-MM-dd hh:mm:ss")));

        m_path[parent] = path;
    }
}

myRemoteDirModel::~myRemoteDirModel()
{

}

void myRemoteDirModel::onItemChanged(QStandardItem *item)
{
    if (item == nullptr)
    {
        return;
    }
    if (item->isCheckable())
    {
        //如果条目是存在复选框的，那么就进行下面的操作
        Qt::CheckState state = item->checkState(); //获取当前的选择状态
        if (!item->isTristate())
        {
            //如果条目是三态的，说明可以对子目录进行全选和全不选的设置
            if (state != Qt::PartiallyChecked)
            {
                //当前是选中状态，需要对其子项目进行全选
                checkAllChild(item, state == Qt::Checked);
            }
        }
        else
        {
            //说明是两态的，两态会对父级的三态有影响
            //判断兄弟节点的情况
            checkChildChanged(item);
        }
    }

    //qDebug() << getCheckedInfo();
}

void myRemoteDirModel::checkAllChild(QStandardItem *item, bool check)
{
    if (item == nullptr)
    {
        return;
    }
    int rowCount = item->rowCount();
    for (int i = 0; i < rowCount; ++i)
    {
        QStandardItem *childItems = item->child(i);
        checkAllChild(childItems, check);
    }
    if (item->isCheckable())
    {
        //qDebug() << check;
        item->setCheckState(check ? Qt::Checked : Qt::Unchecked);
        if(check)
        {
            m_checkedItem.insert(item);
        }
        else
        {
            m_checkedItem.remove(item);
        }
    }
}

void myRemoteDirModel::checkChildChanged(QStandardItem *item)
{
    if (nullptr == item)
    {
        return;
    }
    Qt::CheckState siblingState = checkSibling(item);
    QStandardItem *parentItem = item->parent();
    if (nullptr == parentItem)
    {
        return;
    }
    if (Qt::PartiallyChecked == siblingState)
    {
        if (parentItem->isCheckable() && parentItem->isTristate())
        {
            parentItem->setCheckState(Qt::PartiallyChecked);
        }
    }
    else if (Qt::Checked == siblingState)
    {
        if (parentItem->isCheckable())
        {
            parentItem->setCheckState(Qt::Checked);
        }
    }
    else
    {
        if (parentItem->isCheckable())
        {
            parentItem->setCheckState(Qt::Unchecked);
        }
    }
    checkChildChanged(parentItem);
}

Qt::CheckState myRemoteDirModel::checkSibling(QStandardItem *item)
{
//先通过父节点获取兄弟节点
    QStandardItem *parent = item->parent();
    if (nullptr == parent)
    {
        return item->checkState();
    }
    int brotherCount = parent->rowCount();
    int checkedCount(0), unCheckedCount(0);
    Qt::CheckState state;
    for (int i = 0; i < brotherCount; ++i)
    {
        QStandardItem *siblingItem = parent->child(i);
        state = siblingItem->checkState();
        if (Qt::PartiallyChecked == state)
        {
            return Qt::PartiallyChecked;
        }
        else if (Qt::Unchecked == state)
        {
            ++unCheckedCount;
        }
        else
        {
            ++checkedCount;
        }
        if (checkedCount > 0 && unCheckedCount > 0)
        {
            return Qt::PartiallyChecked;
        }
    }
    if (unCheckedCount > 0)
    {
        return Qt::Unchecked;
    }
    return Qt::Checked;
}

QStringList myRemoteDirModel::getCheckedInfo()
{
    QStringList ret;
    for(const auto &item : m_checkedItem)
    {
        if(item->data(Qt::CheckStateRole) == Qt::Checked)
        {
            if(!m_path[item].isEmpty())
            {
                ret << m_path[item];
            }
        }
    }
    return ret;
}

void myRemoteDirModel::setRootDir(QString root, const QStringList &filter)
{
    reset();
    m_root = root.left(root.lastIndexOf(QDir::separator()));
    m_filter = filter;
}

void myRemoteDirModel::reset()
{
    m_checkedItem.clear();
    m_prefix.clear();
    m_root.clear();
    m_path.clear();
    m_model->clear();
    m_model->setHorizontalHeaderLabels(
            QStringList() << QStringLiteral("名称") << QStringLiteral("大小") << QStringLiteral("类型")
                          << QStringLiteral("修改日期"));
}

bool myRemoteDirModel::matchFilter(const QString &name)
{
    QRegExp rx;
    rx.setPatternSyntax(QRegExp::Wildcard);
    for(const auto& item : m_filter)
    {
        rx.setPattern(item);
        if(rx.exactMatch(name))
        {
            return true;
        }
    }
    return false;
}

void myRemoteDirModel::filterAllItem(QStandardItem *item)
{
    if (item == nullptr)
    {
        return;
    }
    int rowCount = item->rowCount();
    for (int i = 0; i < rowCount; ++i)
    {
        QStandardItem *childItems = item->child(i);
        filterAllItem(childItems);
    }


    if(matchFilter(item->text()))
    {
        item->setData(Qt::Checked, Qt::CheckStateRole);
        m_checkedItem.insert(item);
        item->setEnabled(true);
    }
    else
    {
        item->setData(Qt::Unchecked, Qt::CheckStateRole);
        m_checkedItem.remove(item);
        item->setEnabled(false);
    }
}

void myRemoteDirModel::setFilter(const QStringList &filter)
{
    m_filter = filter;
    filterAllItem(m_model->invisibleRootItem());
}
