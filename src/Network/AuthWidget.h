#ifndef NETWORK_AUTHWIDGET_H
#define NETWORK_AUTHWIDGET_H

#include <QWidget>
#include "ui_AuthWidget.h"

class Server;

class AuthWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit AuthWidget(QWidget* parent = nullptr);
    ~AuthWidget() override;

    void setServer(Server* server);

signals:
    void authComplete();

private:
    Ui_AuthWidget m_ui;
    Server* m_server;

    void onServerStateChanged();

    Q_SLOT void on_googleButton_clicked();
    Q_SLOT void on_cancelButton_clicked();

    Q_DISABLE_COPY_MOVE(AuthWidget)
};

#endif
