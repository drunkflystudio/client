#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <QWebSocket>

class QTimer;

#ifdef WASM_TARGET
class IFrameWindow;
#else
class QOAuth2AuthorizationCodeFlow;
#endif

class Server final : public QObject
{
    Q_OBJECT

public:
    explicit Server(QObject* parent = nullptr);
    ~Server() override;

    bool isAuthenticated() const { return m_authenticated; }
  #ifdef WASM_TARGET
    bool isAuthenticating() const { return m_authFlow != nullptr; }
  #else
    bool isAuthenticating() const { return m_authFlow != nullptr; }
  #endif
    const QString& authError() const { return m_authError; }
    void authenticateWithGoogle();
    void cancelAuthentication();

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
  #ifdef WASM_TARGET
    IFrameWindow* m_authWindow;
  #else
    QOAuth2AuthorizationCodeFlow* m_authFlow;
  #endif
    QTimer* m_reconnectTimer;
    QTimer* m_updateTimer;
    QString m_error;
    QString m_authError;
    QString m_status;
    int m_statusTime;
    int m_reconnectWait;
    bool m_authenticated;
    bool m_connected;

    void stopReconnectTimer();

    void setAuthError(const QString& error);
    void destroyAuthFlow();

    void updateReconnectStatus();

    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString& message);

    Q_DISABLE_COPY_MOVE(Server)
};

#endif
