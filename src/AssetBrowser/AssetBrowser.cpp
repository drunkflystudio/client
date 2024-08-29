#include "AssetBrowser.h"
#include "Network/Server.h"

AssetBrowser::AssetBrowser(QWidget* parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
}

AssetBrowser::~AssetBrowser()
{
}

void AssetBrowser::setServer(Server* server)
{
    m_server = server;
}
