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
    QString state = m_server->state()->statusText();
    QString error = m_server->lastError();
    if (!error.isEmpty())
        state = QStringLiteral("<div style='color:red;font-weight:bold'>%1</div><br>%2").arg(error).arg(state);
    m_ui.statusLabel->setText(state);
    m_ui.reconnectNowButton->setVisible(m_server->state()->id() == Server::WaitingReconnect);
}

void ConnectingWidget::on_reconnectNowButton_clicked()
{
    m_server->reconnect();
}
