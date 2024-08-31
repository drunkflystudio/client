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
    enum StateID
    {
        Offline,
        Authenticating,
        Connecting,
        Online,
        WaitingReconnect,
    };

    class State : public QObject
    {
    public:
        State(Server* server, StateID id) : m_server(server), m_id(id) {}
        virtual ~State() = default;
        virtual QString statusText() const = 0;
        StateID id() const { return m_id; }
    protected:
        Server* const m_server;
        StateID m_id;
        void openUrl(const QString& url);
        Q_DISABLE_COPY_MOVE(State)
    };

    struct Auth
    {
        QString googleToken;
    };

    explicit Server(QObject* parent = nullptr);
    ~Server() override;

    State* state() const { return m_state; }
    const QString& lastError() const { return m_lastError; }

    void authenticateWithGoogle();
    void reconnect();
    void reconnectLater(const QString& error);
    void abortConnection(const QString& error);

signals:
    void stateChanged();

private:
    State* m_state;
    QString m_lastError;
    int m_reconnectWait;
    Auth m_auth;

    void openConnection();
    void setState(State* state);

    Q_DISABLE_COPY_MOVE(Server)
};

#endif
