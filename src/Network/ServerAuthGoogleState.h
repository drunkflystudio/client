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
        , m_state(WaitingCookieState)
    {
        m_query = new QNetworkAccessManager(this);
        m_request.setUrl(QStringLiteral("https://auth.drunkfly.eu/google_status.php"));
        m_reply = m_query->get(m_request);
        connect(m_reply, &QNetworkReply::readyRead, this, &ServerAuthGoogleState::onResponseReceived);
    }

    ~ServerAuthGoogleState() override
    {
        m_reply->deleteLater();
        m_query->deleteLater();
        closeUrl();
    }

    QString statusText() const override
    {
        return tr("Authenticating with Google...");
    }

private:
    enum AuthState
    {
        Error,
        WaitingCookieState,
        WaitingAuth,
        Finished,
    };

    QNetworkAccessManager* m_query;
    QNetworkReply* m_reply;
    QNetworkRequest m_request;
    QString m_cookie;
    QByteArray m_buffer;
    std::function<void(const QString&)> m_onSuccess;
    AuthState m_state;

    void error(const QString& message)
    {
        m_state = Error;
        if (m_server->state() == this)
            m_server->abortConnection(message);
    }

    void onResponseReceived()
    {
        for (;;) {
            if (m_reply->error() != QNetworkReply::NoError) {
                error(tr("Authentication failed: %1").arg(m_reply->errorString()));
                return;
            }

            qint64 bytesAvailable = m_reply->bytesAvailable();
            if (bytesAvailable <= 0)
                break;

            m_buffer += m_reply->read(bytesAvailable);
        }

        switch (m_state) {
            case WaitingCookieState: {
                int index = m_buffer.indexOf('~');
                if (index < 0)
                    return;
                QByteArray json = m_buffer;
                if (index == json.length() - 1) {
                    m_buffer = QByteArray();
                    json.removeLast();
                } else {
                    m_buffer.remove(0, index);
                    json.remove(index - 1, m_buffer.length() - index + 1);
                }
                onCookieReceived(json);
                break;
            }

            case WaitingAuth: {
                if (m_reply->atEnd())
                    onResultReceived(m_buffer);
                break;
            }
        }
    }

    void onCookieReceived(const QByteArray& json)
    {
        auto doc = QJsonDocument::fromJson(json, nullptr);
        if (doc.isNull()) {
            error(tr("Unable to parse response from the server."));
            return;
        }

        auto root = doc.object();
        m_cookie = root[QStringLiteral("cookie")].toString().trimmed();
        if (m_cookie.isEmpty()) {
            error(tr("Unable to parse response from the server."));
            return;
        }

        m_state = WaitingAuth;
        openUrl(QStringLiteral("https://auth.drunkfly.eu/google.php?cookie=%1").arg(m_cookie));
    }

    void onResultReceived(const QByteArray& json)
    {
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

        auto email = root[QStringLiteral("email")].toString().trimmed();
        auto token = root[QStringLiteral("token")].toString().trimmed();
        if (email.isEmpty() || token.isEmpty()) {
            error(tr("Unable to parse response from the server."));
            return;
        }

        m_state = Finished;
        if (m_server->state() == this)
            m_onSuccess(token);
    }

    Q_DISABLE_COPY_MOVE(ServerAuthGoogleState)
};
