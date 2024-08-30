#include "AssetTreeItem.h"

AssetTreeItem::AssetTreeItem(const QString& name, AssetTreeItem* parent)
    : m_parent(parent)
    , m_name(name)
{
}

int AssetTreeItem::indexInParent() const
{
    if (!m_parent)
        return 0;

    const auto begin = m_parent->m_children.cbegin();
    const auto end = m_parent->m_children.cend();
    const auto it = std::find_if(begin, end, [this](const auto& item) { return item.get() == this; });
    if (it != end)
        return std::distance(begin, it);

    Q_ASSERT(false);
    return -1;
}

AssetTreeItem* AssetTreeItem::child(int index) const
{
    if (index < 0 || index >= int(m_children.size()))
        return nullptr;

    return m_children[size_t(index)].get();
}
