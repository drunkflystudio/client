#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <functional>

class ServerValidateSessionState final : public Server::State
{
    Q_OBJECT

public:
    explicit ServerValidateSessionState(Server* server, const QString& sessID,
            std::function<void()> onSuccess, std::function<void()> onFailure)
        : State(server, Server::Authenticating)
        , m_onSuccess(onSuccess)
        , m_onFailure(onFailure)
        , m_callbackCalled(false)
    {
        m_query = new QNetworkAccessManager(this);
        m_request.setUrl(QStringLiteral("https://auth.drunkfly.eu/session_get.php?id=%1").arg(sessID));
        m_reply = m_query->get(m_request);
        connect(m_reply, &QNetworkReply::finished, this, &ServerValidateSessionState::onResponseReceived);
        connect(m_reply, &QNetworkReply::errorOccurred, this, &ServerValidateSessionState::onError);
    }

    ~ServerValidateSessionState() override
    {
        m_reply->deleteLater();
        m_query->deleteLater();
    }

    QString statusText() const override
    {
        return tr("Checking user credentials...");
    }

private:
    QNetworkAccessManager* m_query;
    QNetworkReply* m_reply;
    QNetworkRequest m_request;
    std::function<void()> m_onSuccess;
    std::function<void()> m_onFailure;
    bool m_callbackCalled;

    void onError()
    {
        if (!m_callbackCalled) {
            m_callbackCalled = true;
            if (m_server->state() == this)
                m_onFailure();
        }
    }

    void onResponseReceived()
    {
        if (m_reply->error() != QNetworkReply::NoError) {
            onError();
            return;
        }

        auto json = m_reply->readAll();
        auto doc = QJsonDocument::fromJson(json, nullptr);
        if (doc.isNull()) {
            onError();
            return;
        }

        auto root = doc.object();
        auto sessionValid = root[QStringLiteral("session_valid")].toBool();
        if (!sessionValid) {
            onError();
            return;
        }

        if (!m_callbackCalled) {
            m_callbackCalled = true;
            if (m_server->state() == this)
                m_onSuccess();
        }
    }

    Q_DISABLE_COPY_MOVE(ServerValidateSessionState)
};
