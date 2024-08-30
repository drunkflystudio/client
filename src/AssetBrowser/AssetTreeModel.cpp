#include "AssetTreeModel.h"
#include "AssetBrowser/AssetTreeItem.h"

AssetTreeModel::AssetTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
{
}

AssetTreeModel::~AssetTreeModel()
{
}

int AssetTreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() > 0)
        return 0;

    const AssetTreeItem* parentItem = treeItem(parent);
    if (!parentItem)
        return 0;

    return parentItem->childCount();
}

int AssetTreeModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QModelIndex AssetTreeModel::parent(const QModelIndex& index) const
{
    if (index.isValid()) {
        AssetTreeItem* childItem = treeItem(index);
        if (childItem) {
            AssetTreeItem* parentItem = childItem->parent();
            if (parentItem != m_root.get())
                return createIndex(parentItem->indexInParent(), 0, parentItem);
        }
    }

    return QModelIndex();
}

QModelIndex AssetTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    AssetTreeItem* parentItem = treeItem(parent);
    if (!parentItem)
        return QModelIndex();

    if (auto child = parentItem->child(row))
        return createIndex(row, column, child);

    return QModelIndex();
}

QVariant AssetTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.column() != 0)
        return QModelIndex();

    AssetTreeItem* item = treeItem(index);
    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return item->name();
    }

    return QModelIndex();
}

AssetTreeItem* AssetTreeModel::treeItem(const QModelIndex& index) const
{
    if (index.isValid()) {
        if (auto item = reinterpret_cast<AssetTreeItem*>(index.internalPointer()))
            return item;
    }

    return m_root.get();
}
