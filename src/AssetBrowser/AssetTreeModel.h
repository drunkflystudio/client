#ifndef ASSETBROWSER_ASSETTREEMODEL_H
#define ASSETBROWSER_ASSETTREEMODEL_H

#include <QAbstractItemModel>
#include <memory>

class AssetTreeItem;

class AssetTreeModel final : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit AssetTreeModel(QObject* parent = nullptr);
    ~AssetTreeModel() override;

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;

    QModelIndex parent(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = {}) const override;

    QVariant data(const QModelIndex& index, int role) const override;

private:
    std::unique_ptr<AssetTreeItem> m_root;

    AssetTreeItem* treeItem(const QModelIndex& index) const;

    Q_DISABLE_COPY_MOVE(AssetTreeModel)
};

#endif
