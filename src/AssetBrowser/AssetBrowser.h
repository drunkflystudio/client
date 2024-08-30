#ifndef ASSETBROWSER_ASSETBROWSER_H
#define ASSETBROWSER_ASSETBROWSER_H

#include <QWidget>
#include "ui_AssetBrowser.h"

class Server;
class AssetTreeModel;

class AssetBrowser final : public QWidget
{
    Q_OBJECT

public:
    explicit AssetBrowser(QWidget* parent = nullptr);
    ~AssetBrowser() override;

    void setServer(Server* server);

private:
    Ui_AssetBrowser m_ui;
    Server* m_server;
    AssetTreeModel* m_assetTreeModel;

    Q_DISABLE_COPY_MOVE(AssetBrowser)
};

#endif
