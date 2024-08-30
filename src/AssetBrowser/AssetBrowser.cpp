#include "AssetBrowser.h"
#include "AssetBrowser/AssetTreeModel.h"
#include "Network/Server.h"

AssetBrowser::AssetBrowser(QWidget* parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
    m_ui.assetTree->setModel(m_assetTreeModel = new AssetTreeModel(this));
}

AssetBrowser::~AssetBrowser()
{
}

void AssetBrowser::setServer(Server* server)
{
    m_server = server;
}
