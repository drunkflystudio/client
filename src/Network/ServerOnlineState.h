#include "Connection.h"
#include <QWebSocket>

class Connection;

class ServerOnlineState final : public Server::State
{
    Q_OBJECT

public:
    explicit ServerOnlineState(Server* server, QWebSocket* socket)
        : State(server, Server::Online)
        , m_socket(socket)
    {
        m_connection = new Connection(m_socket);
        connect(m_connection, &Connection::onServerError, this, &ServerOnlineState::onServerError);
        connect(m_connection, &Connection::onSerializationError, this, &ServerOnlineState::onSerializationError);
        connect(m_connection, &Connection::onProtocolError, this, &ServerOnlineState::onProtocolError);
        connect(m_socket, &QWebSocket::disconnected, this, &ServerOnlineState::onDisconnected);
    }

    ~ServerOnlineState() override
    {
        m_connection->deleteLater();
        m_socket->deleteLater();
    }

    QString statusText() const override
    {
        return tr("Connected");
    }

private:
    QWebSocket* const m_socket;
    Connection* m_connection;

    void onDisconnected()
    {
        if (m_server->state() == this)
            m_server->reconnectLater(tr("Connection closed by server."));
    }

    void onServerError()
    {
        if (m_server->state() == this)
            m_server->reconnectLater(tr("Server was unable to handle request."));
    }

    void onSerializationError()
    {
        if (m_server->state() == this)
            m_server->reconnectLater(tr("Server has sent invalid or incomplete data packet."));
    }

    void onProtocolError()
    {
        if (m_server->state() == this)
            m_server->reconnectLater(tr("Server has sent unexpected data packet."));
    }

    Q_DISABLE_COPY_MOVE(ServerOnlineState)
};
