#include <QWebSocket>

class ServerOnlineState final : public Server::State
{
    Q_OBJECT

public:
    explicit ServerOnlineState(Server* server, QWebSocket* socket)
        : State(server, Server::Online)
        , m_socket(socket)
    {
        connect(m_socket, &QWebSocket::disconnected, this, &ServerOnlineState::onDisconnected);
    }

    ~ServerOnlineState() override
    {
        m_socket->deleteLater();
    }

    QString statusText() const override
    {
        return tr("Connected");
    }

private:
    QWebSocket* const m_socket;

    void onDisconnected()
    {
        if (m_server->state() == this)
            m_server->reconnectLater(tr("Connection closed by server."));
    }

    Q_DISABLE_COPY_MOVE(ServerOnlineState)
};
