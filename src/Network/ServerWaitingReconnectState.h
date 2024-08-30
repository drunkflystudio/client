#include <QTimer>
#include <QElapsedTimer>

class ServerWaitingReconnectState final : public Server::State
{
    Q_OBJECT

public:
    ServerWaitingReconnectState(Server* server, int milliseconds)
        : State(server, Server::WaitingReconnect)
        , m_timeout(milliseconds)
        , m_timeLeft((milliseconds + 999) / 1000)
    {
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &ServerWaitingReconnectState::onTimer);
        m_timeCounter.start();
        m_timer->start(100);
    }

    QString statusText() const override
    {
        return tr("Will reconnect in %1s...").arg(m_timeLeft);
    }

private:
    QTimer* m_timer;
    QElapsedTimer m_timeCounter;
    int m_timeout;
    int m_timeLeft;

    void onTimer()
    {
        int elapsedMilliseconds = m_timeCounter.elapsed();
        if (elapsedMilliseconds >= m_timeout) {
            m_timer->stop();
            if (m_server->state() == this)
                m_server->reconnect();
            return;
        }

        int timeLeft = (m_timeout - elapsedMilliseconds + 999) / 1000;
        if (m_timeLeft != timeLeft) {
            m_timeLeft = timeLeft;
            if (m_server->state() == this)
                emit m_server->stateChanged();
        }
    }

    Q_DISABLE_COPY_MOVE(ServerWaitingReconnectState)
};
