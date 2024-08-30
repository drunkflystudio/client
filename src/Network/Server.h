#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <QWebSocket>

class QTimer;

class Server final : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);
    ~Server() override;

    bool isConnected() const { return m_connected; }
    bool isConnecting() const { return !m_connected && (m_reconnectTimer || m_socket); }
    bool isConnectionPending() const { return m_reconnectTimer; }
    const QString& statusText() const { return m_status; }

    void openConnection();
    void openNewConnection();
    void closeConnection();

signals:
    void stateChanged();

private:
    QWebSocket* m_socket;
    QTimer* m_reconnectTimer;
    QTimer* m_updateTimer;
    QString m_error;
    QString m_status;
    int m_statusTime;
    int m_reconnectWait;
    bool m_connected;

    void stopReconnectTimer();

    void updateReconnectStatus();

    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);

    Q_DISABLE_COPY_MOVE(Server)
};

#endif
