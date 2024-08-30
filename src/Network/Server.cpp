#include "Server.h"
#include <QTimer>

const int InitialReconnectWait = 3000;
const int MaxReconnectWait = 30000;
const int ReconnectWaitGrowth = 2000;

Server::Server(QObject* parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_reconnectTimer(nullptr)
    , m_status(tr("Offline"))
    , m_statusTime(-1)
    , m_reconnectWait(InitialReconnectWait)
    , m_authenticated(false)
    , m_authenticating(false)
    , m_connected(false)
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(100);
    connect(m_updateTimer, &QTimer::timeout, this, &Server::updateReconnectStatus);
}

Server::~Server()
{
    stopReconnectTimer();
    closeConnection();
}

void Server::authenticate()
{
    // FIXME
    m_authenticating = true;
    emit stateChanged();

    QTimer::singleShot(3000, [this]() {
            m_authenticating = false;

            m_authenticated = true;
            m_reconnectWait = InitialReconnectWait;
            openConnection();
        });
}

void Server::openConnection()
{
    if (m_socket)
        return;

    stopReconnectTimer();

    if (m_authenticating || !m_authenticated) {
        closeConnection();
        return;
    }

    m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    connect(m_socket, &QWebSocket::connected, this, &Server::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &Server::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &Server::onTextMessageReceived);

    m_status = tr("Connecting...");
    emit stateChanged();

    m_socket->open(QStringLiteral("ws://127.0.0.1:8080/"));
}

void Server::openNewConnection()
{
    closeConnection();
    openConnection();
}

void Server::closeConnection()
{
    stopReconnectTimer();

    if (!m_socket)
        return;

    m_socket->disconnect(this);
    m_socket->deleteLater();
    m_socket = nullptr;

    m_status = tr("Offline");
    m_connected = false;
    emit stateChanged();
}

void Server::stopReconnectTimer()
{
    m_updateTimer->stop();

    if (m_reconnectTimer) {
        m_reconnectTimer->disconnect(this);
        m_reconnectTimer->deleteLater();
        m_reconnectTimer = nullptr;
    }
}

void Server::updateReconnectStatus()
{
    int timeLeft = -1;
    if (m_reconnectTimer)
        timeLeft = m_reconnectTimer->remainingTime();

    if (m_statusTime == timeLeft)
        return;

    m_statusTime = timeLeft;
    if (timeLeft <= 0) {
        m_updateTimer->stop();
        return;
    }

    m_status = tr("Will reconnect in %1s...").arg((timeLeft + 999) / 1000);
    if (!m_error.isEmpty())
        m_status = QStringLiteral("<div style='color:red;font-weight:bold'>%1</div><br>%2").arg(m_error).arg(m_status);

    emit stateChanged();
}

void Server::onConnected()
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    if (socket != m_socket)
        return;

    stopReconnectTimer();

    m_connected = true;
    m_reconnectWait = InitialReconnectWait;
    m_status = tr("Connected");
    emit stateChanged();

    // FIXME
    socket->sendTextMessage(QStringLiteral("Hello, world!"));
}

void Server::onDisconnected()
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    if (socket != m_socket)
        return;

    m_error = m_socket->errorString();

    socket->disconnect(this);
    socket->deleteLater();

    m_connected = false;
    m_socket = nullptr;

    stopReconnectTimer();

    if (!m_authenticated)
        return;

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    m_reconnectTimer->callOnTimeout(this, &Server::openConnection);
    m_reconnectTimer->start(m_reconnectWait);

    m_reconnectWait += ReconnectWaitGrowth;
    if (m_reconnectWait > MaxReconnectWait)
        m_reconnectWait = MaxReconnectWait;

    m_statusTime = -2;
    m_updateTimer->start();
    updateReconnectStatus();
}

void Server::onTextMessageReceived(const QString& message)
{
    // FIXME
}
