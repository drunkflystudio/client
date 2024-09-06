#include "Network/Protocol.h"
#include <QWebSocket>
#include <QTimer>
#include <functional>

class ServerConnectingState final : public Server::State
{
    Q_OBJECT

public:
    ServerConnectingState(Server* server, const QString& sessID, std::function<void(QWebSocket*)> onConnected)
        : State(server, Server::Connecting)
        , m_onConnected(onConnected)
        , m_ownSocket(true)
    {
        m_timer = new QTimer(this);
        m_timer->setSingleShot(true);
        connect(m_timer, &QTimer::timeout, this, &ServerConnectingState::onTimeout);
        m_timer->start(10000);

        m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, m_server);
        connect(m_socket, &QWebSocket::textMessageReceived, this, &ServerConnectingState::onTextMessageReceived);
        connect(m_socket, &QWebSocket::disconnected, this, &ServerConnectingState::onDisconnected);
        m_socket->open(QStringLiteral("ws://127.0.0.1/?sid=%1").arg(sessID.toLatin1()));
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
    QTimer* m_timer;
    std::function<void(QWebSocket*)> m_onConnected;
    bool m_ownSocket;

    void onTextMessageReceived(const QString& message)
    {
        m_ownSocket = false;
        m_socket->disconnect(this);
        m_timer->stop();

        if (message.startsWith(QStringLiteral("PROTOv"))) {
            int version = message.mid(6, message.length() - 6).toInt();
            if (version != Protobuf::ConstantsGadget::ProtocolVersion) {
                if (m_server->state() == this) {
                    m_server->abortConnectionAndLogout(
                        tr("Server uses unsupported protocol version %1 (expected version %2). Please update your client software.")
                        .arg(version).arg(Protobuf::ConstantsGadget::ProtocolVersion));
                }
                return;
            }

            if (m_server->state() == this)
                m_onConnected(m_socket);
            else
                m_socket->deleteLater();

            return;
        }

        if (m_server->state() == this)
            m_server->abortConnectionAndLogout(tr("Session expired, please login again."));
    }

    void onTimeout()
    {
        if (m_server->state() == this)
            m_server->reconnectLater(tr("Connection timed out."));
    }

    void onDisconnected()
    {
        m_ownSocket = false;
        m_socket->deleteLater();
        m_timer->stop();

        if (m_server->state() == this)
            m_server->reconnectLater(tr("Connection failed."));
    }

    Q_DISABLE_COPY_MOVE(ServerConnectingState)
};
