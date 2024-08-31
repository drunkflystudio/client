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
    bool authenticating = m_server->state()->id() == Server::Authenticating;
    if (authenticating) {
        m_ui.statusLabel->setVisible(true);
        m_ui.statusLabel->setText(m_server->state()->statusText());
    } else if (!m_server->lastError().isEmpty()) {
        m_ui.statusLabel->setVisible(true);
        m_ui.statusLabel->setText(
            QStringLiteral("<div style='color:red;font-weight:bold'>%1</div>").arg(m_server->lastError()));
    } else
        m_ui.statusLabel->setVisible(false);

    m_ui.googleButton->setVisible(!authenticating);
    m_ui.cancelButton->setVisible(authenticating);
}

void AuthWidget::on_googleButton_clicked()
{
    m_server->authenticateWithGoogle();
}

void AuthWidget::on_cancelButton_clicked()
{
    m_server->abortConnection(tr("Authentication aborted by user."));
}
