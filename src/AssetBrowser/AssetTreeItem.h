#ifndef ASSETBROWSER_ASSETTREEITEM_H
#define ASSETBROWSER_ASSETTREEITEM_H

#include <QString>
#include <memory>
#include <vector>

class AssetTreeItem final
{
public:
    explicit AssetTreeItem(const QString& name, AssetTreeItem* parent = nullptr);

    const QString& name() const { return m_name; }

    AssetTreeItem* parent() const { return m_parent; }
    int indexInParent() const;

    int childCount() const { return int(m_children.size()); }
    AssetTreeItem* child(int index) const;

private:
    AssetTreeItem* m_parent;
    QString m_name;
    std::vector<std::unique_ptr<AssetTreeItem>> m_children;
};

#endif
