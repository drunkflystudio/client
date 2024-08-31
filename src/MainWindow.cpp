#include "MainWindow.h"
#include "Network/ConnectingWidget.h"
#include "Network/Server.h"
#include <QMessageBox>

#ifdef WASM_TARGET
#include <emscripten.h>
EM_JS(void, JSSetDocumentTitle, (const char* title), { document.title = UTF8ToString(title); });
#endif

MainWindow::MainWindow(Server* server, QWidget* parent)
    : QMainWindow(parent)
    , m_server(server)
{
    m_ui.setupUi(this);
    m_ui.connectingWidget->setServer(m_server);
    m_ui.authWidget->setServer(m_server);
    m_ui.assetBrowser->setServer(m_server);
    updateTitle();

    onServerStateChanged();
    connect(m_server, &Server::stateChanged, this, &MainWindow::onServerStateChanged);
}

MainWindow::~MainWindow()
{
}

void MainWindow::updateTitle()
{
    QString title = QApplication::applicationName();
    setWindowTitle(title);

  #ifdef WASM_TARGET
    JSSetDocumentTitle(title.toUtf8().constData());
  #endif
}

void MainWindow::onServerStateChanged()
{
    QWidget* page = nullptr;

    switch (m_server->state()->id()) {
        case Server::Offline:
        case Server::Authenticating:
            page = m_ui.authWidget; break;
        case Server::Connecting:
        case Server::WaitingReconnect:
            page = m_ui.connectingWidget;
            break;
        case Server::Online:
            page = (QWidget*)m_ui.contents;
            break;
    }

    m_ui.menuBar->setVisible(m_server->state()->id() == Server::Online);
    m_ui.statusBar->setVisible(m_server->state()->id() == Server::Online);
    m_ui.centralWidget->setCurrentWidget(page);
}

void MainWindow::on_actionLogout_triggered()
{
    auto server = m_server;

    QMessageBox* msgbox = new QMessageBox(QMessageBox::Question,
        tr("Confirmation"),
        tr("Do you really want to log out?"),
        QMessageBox::NoButton,
        this);
    auto logoutButton = msgbox->addButton(tr("Log &out"), QMessageBox::DestructiveRole);
    auto cancelButton = msgbox->addButton(tr("&Cancel"), QMessageBox::RejectRole);
    msgbox->setEscapeButton(cancelButton);
    msgbox->setDefaultButton(cancelButton);
    msgbox->setModal(true);
    connect(server, &Server::stateChanged, msgbox, [msgbox, server] {
            if (server->state()->id() != Server::Online)
                msgbox->reject();
        });
    connect(msgbox, &QMessageBox::done, msgbox, [msgbox](int r){ msgbox->deleteLater(); });
    connect(msgbox, &QMessageBox::buttonClicked, this, [this, logoutButton](QAbstractButton* button) {
            if (button == logoutButton)
                m_server->abortConnectionAndLogout();
        });
    msgbox->show();
}
