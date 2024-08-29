#include "MainWindow.h"
#include "Network/ConnectingWidget.h"
#include "Network/Server.h"

MainWindow::MainWindow(Server* server, QWidget* parent)
    : QMainWindow(parent)
    , m_server(server)
{
    m_ui.setupUi(this);
    m_ui.connectingWidget->setServer(m_server);

    onServerStateChanged();
    connect(m_server, &Server::stateChanged, this, &MainWindow::onServerStateChanged);
}

MainWindow::~MainWindow()
{
}

void MainWindow::onServerStateChanged()
{
    m_ui.centralWidget->setCurrentWidget(m_server->isConnected() ? m_ui.contents : m_ui.connectingWidget);
}
