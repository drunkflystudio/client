#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QUuid>
#include <QTimer>
#include <functional>

class ServerAuthGoogleState final : public Server::State
{
    Q_OBJECT

public:
    explicit ServerAuthGoogleState(Server* server, std::function<void(const QString&)> onSuccess)
        : State(server, Server::Authenticating)
        , m_query(nullptr)
        , m_onSuccess(onSuccess)
    {
        m_cookie = QUuid::createUuid().toString(QUuid::Id128);

        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &ServerAuthGoogleState::onTimer);
        m_timer->start(2000);

        openUrl(QStringLiteral("https://auth.drunkfly.eu/google.php?cookie=%1").arg(m_cookie));
    }

    ~ServerAuthGoogleState() override
    {
        if (m_query)
            m_query->deleteLater();
    }

    QString statusText() const override
    {
        return tr("Authenticating with Google...");
    }

private:
    QTimer* m_timer;
    QNetworkAccessManager* m_query;
    QNetworkRequest m_request;
    QString m_cookie;
    std::function<void(const QString&)> m_onSuccess;

    void onTimer()
    {
        if (!m_query)
            sendRequest();
    }

    void sendRequest()
    {
        m_query = new QNetworkAccessManager(this);
        connect(m_query, &QNetworkAccessManager::finished, this, &ServerAuthGoogleState::onStatusReceived);

        m_request = QNetworkRequest();
        m_request.setUrl(QStringLiteral("https://auth.drunkfly.eu/google_status.php?cookie=%1").arg(m_cookie));
        m_query->get(m_request);
    }

    void onStatusReceived(QNetworkReply* reply)
    {
        reply->deleteLater();

        if (m_query) {
            m_query->deleteLater();
            m_query = nullptr;
        }

        if (reply->error() != QNetworkReply::NoError) {
            m_timer->stop();
            if (m_server->state() == this)
                m_server->abortConnection(reply->errorString());
            return;
        }

        auto response = reply->readAll();
        auto json = QJsonDocument::fromJson(response, nullptr);
        if (json.isNull()) {
            m_timer->stop();
            if (m_server->state() == this)
                m_server->abortConnection(tr("Unable to parse response from the server."));
            return;
        }

        auto root = json.object();
        QString status = root[QStringLiteral("status")].toString();
        if (status == QStringLiteral("waiting"))
            return;
        if (status == QStringLiteral("invalid_token")) {
            m_timer->stop();
            if (m_server->state() == this)
                m_server->abortConnection(tr("Authentication timed out."));
            return;
        }
        if (status != QStringLiteral("accepted")) {
            m_timer->stop();
            if (m_server->state() == this)
                m_server->abortConnection(tr("Authentication failed."));
            return;
        }

        QString token = root[QStringLiteral("token")].toString();
        if (token.isEmpty()) {
            m_timer->stop();
            if (m_server->state() == this)
                m_server->abortConnection(tr("Authentication failed."));
            return;
        }

        m_timer->stop();
        if (m_server->state() == this)
            m_onSuccess(token);
    }

    Q_DISABLE_COPY_MOVE(ServerAuthGoogleState)
};
