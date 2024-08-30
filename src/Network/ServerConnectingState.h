#include <QWebSocket>
#include <functional>

class ServerConnectingState final : public Server::State
{
    Q_OBJECT

public:
    ServerConnectingState(Server* server, const QUrl& url, const Server::Auth& auth,
            std::function<void(QWebSocket*)> onConnected)
        : State(server, Server::Connecting)
        , m_auth(auth)
        , m_onConnected(onConnected)
        , m_ownSocket(true)
    {
        m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, m_server);
        connect(m_socket, &QWebSocket::connected, this, &ServerConnectingState::onConnected);
        connect(m_socket, &QWebSocket::disconnected, this, &ServerConnectingState::onDisconnected);
        m_socket->open(url);
    }

    ~ServerConnectingState() override
    {
        if (m_ownSocket)
            m_socket->deleteLater();
    }

    QString statusText() const override
    {
        return tr("Connecting...");
    }

private:
    QWebSocket* m_socket;
    Server::Auth m_auth;
    std::function<void(QWebSocket*)> m_onConnected;
    bool m_ownSocket;

    void onConnected()
    {
        // FIXME
        m_socket->sendTextMessage(m_auth.googleToken);

        m_ownSocket = false;
        m_socket->disconnect(this);

        if (m_server->state() == this)
            m_onConnected(m_socket);
        else
            m_socket->deleteLater();
    }

    void onDisconnected()
    {
        m_ownSocket = false;
        m_socket->deleteLater();
        if (m_server->state() == this)
            m_server->reconnectLater(tr("Connection failed."));
    }

    Q_DISABLE_COPY_MOVE(ServerConnectingState)
};
