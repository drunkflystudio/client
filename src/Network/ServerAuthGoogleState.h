#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <functional>

class ServerAuthGoogleState final : public Server::State
{
    Q_OBJECT

public:
    explicit ServerAuthGoogleState(Server* server, std::function<void(const QString&)> onSuccess)
        : State(server, Server::Authenticating)
        , m_onSuccess(onSuccess)
        , m_state(WaitingHeaders)
    {
        m_query = new QNetworkAccessManager(this);
        m_request.setUrl(QStringLiteral("https://auth.drunkfly.eu/google_status.php"));
        m_reply = m_query->get(m_request);
        connect(m_reply, &QNetworkReply::metaDataChanged, this, &ServerAuthGoogleState::onHeadersReceived);
        connect(m_reply, &QNetworkReply::finished, this, &ServerAuthGoogleState::onResponseReceived);
        connect(m_reply, &QNetworkReply::errorOccurred, this, &ServerAuthGoogleState::onError);
    }

    ~ServerAuthGoogleState() override
    {
        m_reply->deleteLater();
        m_query->deleteLater();
        closeUrl();
    }

    QString statusText() const override
    {
        switch (m_state) {
            case WaitingHeaders: return tr("Requesting authentication cookie...");
            case Finished: case Error: return tr("Finalizing...");
            default: return tr("Waiting for user action...");
        }
    }

private:
    enum AuthState
    {
        Error,
        WaitingHeaders,
        WaitingResponse,
        Finished,
    };

    QNetworkAccessManager* m_query;
    QNetworkReply* m_reply;
    QNetworkRequest m_request;
    QString m_cookie;
    std::function<void(const QString&)> m_onSuccess;
    AuthState m_state;

    void error(const QString& message)
    {
        if (m_state != Error && m_state != Finished) {
            m_state = Error;
            if (m_server->state() == this)
                m_server->abortConnection(message);
        }
    }

    void onError(const QNetworkReply::NetworkError code)
    {
        error(tr("Authentication failed: %1").arg(m_reply->errorString()));
    }

    void onHeadersReceived()
    {
        if (m_state != WaitingHeaders)
            return;

        auto header = m_reply->rawHeader("Content-Type");
        int index = header.indexOf("cookie=");
        if (index < 0) {
            error(tr("Invalid response from the server."));
            return;
        }
        index += 7;
        m_cookie = QString::fromLatin1(header.mid(index, header.length() - index).constData());

        openUrl(QStringLiteral("https://auth.drunkfly.eu/google.php?cookie=%1").arg(m_cookie));

        m_state = WaitingResponse;
        if (m_server)
            emit m_server->stateChanged();
    }

    void onResponseReceived()
    {
        if (m_reply->error() != QNetworkReply::NoError) {
            error(tr("Authentication failed: %1").arg(m_reply->errorString()));
            return;
        }

        auto json = m_reply->readAll();
        auto doc = QJsonDocument::fromJson(json, nullptr);
        if (doc.isNull()) {
            error(tr("Unable to parse response from the server."));
            return;
        }

        auto root = doc.object();
        auto status = root[QStringLiteral("status")].toString().trimmed();
        if (status == "database_error") {
            error(tr("Database failure on the server."));
            return;
        } else if (status == "state_mismatch") {
            error(tr("Authentication process failed."));
            return;
        } else if (status == "invalid_token") {
            error(tr("Invalid authentication token."));
            return;
        } else if (status == "failed") {
            error(tr("Authentication was rejected by Google."));
            return;
        } else if (status != "finished") {
            error(tr("Unable to parse response from the server."));
            return;
        }

        auto session = root[QStringLiteral("session")].toString().trimmed();
        if (session.isEmpty()) {
            error(tr("Unable to parse response from the server."));
            return;
        }

        if (m_state != Error && m_state != Finished) {
            m_state = Finished;
            if (m_server->state() == this)
                m_onSuccess(session);
        }
    }

    Q_DISABLE_COPY_MOVE(ServerAuthGoogleState)
};
