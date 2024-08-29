#include "ConnectingWidget.h"
#include "Network/Server.h"

ConnectingWidget::ConnectingWidget(QWidget* parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
}

ConnectingWidget::~ConnectingWidget()
{
}

void ConnectingWidget::setServer(Server* server)
{
    m_server = server;
    connect(m_server, &Server::stateChanged, this, &ConnectingWidget::onServerStateChanged);
    onServerStateChanged();
}

void ConnectingWidget::onServerStateChanged()
{
    m_ui.statusLabel->setText(m_server->statusText());
    m_ui.reconnectNowButton->setVisible(m_server->isConnectionPending());
}

void ConnectingWidget::on_reconnectNowButton_clicked()
{
    m_server->openConnection();
}
