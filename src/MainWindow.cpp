#include "MainWindow.h"
#include "Network/ConnectingWidget.h"
#include "Network/Server.h"

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

    if (!m_server->isAuthenticated())
        page = m_ui.authWidget;
    else if (!m_server->isConnected())
        page = m_ui.connectingWidget;
    else
        page = (QWidget*)m_ui.contents;

    m_ui.centralWidget->setCurrentWidget(page);
}
