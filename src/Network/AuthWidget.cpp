#include "AuthWidget.h"
#include "Network/Server.h"

AuthWidget::AuthWidget(QWidget* parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
}

AuthWidget::~AuthWidget()
{
}

void AuthWidget::setServer(Server* server)
{
    m_server = server;
    connect(m_server, &Server::stateChanged, this, &AuthWidget::onServerStateChanged);
    onServerStateChanged();
}

void AuthWidget::onServerStateChanged()
{
    setEnabled(!m_server->isAuthenticating());
}

void AuthWidget::on_loginButton_clicked()
{
    m_server->authenticate();
}
