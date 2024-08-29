#ifndef NETWORK_CONNECTINGWIDGET_H
#define NETWORK_CONNECTINGWIDGET_H

#include <QWidget>
#include "ui_ConnectingWidget.h"

class Server;

class ConnectingWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectingWidget(QWidget* parent = nullptr);
    ~ConnectingWidget() override;

    void setServer(Server* server);

private:
    Ui_ConnectingWidget m_ui;
    Server* m_server;

    void onServerStateChanged();

    Q_SLOT void on_reconnectNowButton_clicked();

    Q_DISABLE_COPY(ConnectingWidget)
};

#endif
