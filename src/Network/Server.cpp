#include "Server.h"
#ifdef WASM_TARGET
#else
#include <QOAuthHttpServerReplyHandler>
#include <QOAuth2AuthorizationCodeFlow>
#endif
#include <QUrlQuery>
#include <QDesktopServices>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QTimer>

const int InitialReconnectWait = 3000;
const int MaxReconnectWait = 30000;
const int ReconnectWaitGrowth = 2000;

Server::Server(QObject* parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_authFlow(nullptr)
    , m_reconnectTimer(nullptr)
    , m_status(tr("Offline"))
    , m_statusTime(-1)
    , m_reconnectWait(InitialReconnectWait)
    , m_authenticated(false)
    , m_connected(false)
{
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(100);
    connect(m_updateTimer, &QTimer::timeout, this, &Server::updateReconnectStatus);
}

Server::~Server()
{
    stopReconnectTimer();
    closeConnection();
}

void Server::authenticateWithGoogle()
{
    m_authenticated = false;
    cancelAuthentication();

    stopReconnectTimer();
    closeConnection();

  #ifdef WASM_TARGET
    QFile file(QStringLiteral(":/google.json"));
    if (!file.open(QFile::ReadOnly)) {
        setAuthError(tr("Unable to load client secret: %1").arg(file.errorString()));
        return;
    }
    QJsonParseError error;
    auto json = QJsonDocument::fromJson(file.readAll(), &error);
    if (json.isNull()) {
        setAuthError(tr("Unable to load client secret: %1").arg(error.errorString()));
        return;
    }
    auto clientSecret = json.object()[QStringLiteral("web")].toObject();
    auto port = quint16(QUrl(clientSecret[QStringLiteral("redirect_uris")].toArray()[0].toString()).port());
    file.close();

    m_authFlow = new QOAuth2AuthorizationCodeFlow(this);
    m_authFlow->setScope("email");
    m_authFlow->setAuthorizationUrl(clientSecret[QStringLiteral("auth_uri")].toString());
    m_authFlow->setAccessTokenUrl(clientSecret[QStringLiteral("token_uri")].toString());
    m_authFlow->setClientIdentifier(clientSecret[QStringLiteral("client_id")].toString());
    m_authFlow->setClientIdentifierSharedKey(clientSecret[QStringLiteral("client_secret")].toString());
    m_authFlow->setReplyHandler(new QOAuthHttpServerReplyHandler(port, m_authFlow));

    auto authToken = std::make_shared<QString>();

    m_authFlow->setModifyParametersFunction([](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>* params) {
            if (stage == QAbstractOAuth::Stage::RequestingAccessToken) {
                QByteArray code = params->value(QStringLiteral("code")).toByteArray();
                params->replace(QStringLiteral("code"), QUrl::fromPercentEncoding(code));
            }
        });

    connect(m_authFlow, &QOAuth2AuthorizationCodeFlow::granted, [this, authToken]() {
            if (authToken->isEmpty()) {
                setAuthError(tr("Missing authentication token."));
                return;
            }

            destroyAuthFlow();
            m_authenticated = true;
            m_authError.clear();
            m_reconnectWait = InitialReconnectWait;
            openConnection();
        });

    connect(m_authFlow, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, [this](QUrl url) {
            QUrlQuery query(url);
            query.addQueryItem(QStringLiteral("access_type"), QStringLiteral("offline"));
            url.setQuery(query);
            QDesktopServices::openUrl(url);
        });

    connect(m_authFlow, &QOAuth2AuthorizationCodeFlow::tokenChanged, [authToken](const QString& token) {
            authToken->assign(token);
        });

    connect(m_authFlow, &QOAuth2AuthorizationCodeFlow::requestFailed, [this](QAbstractOAuth::Error error) {
            switch (error) {
                case QAbstractOAuth::Error::NetworkError:
                    setAuthError(tr("Failed to connect to the server."));
                    break;
                case QAbstractOAuth::Error::ServerError:
                    setAuthError(tr("The server answered the request with an error, or its response was not successfully received."));
                    break;
                case QAbstractOAuth::Error::OAuthTokenNotFoundError:
                    setAuthError(tr("The server's response to a token request provided no token identifier."));
                    break;
                case QAbstractOAuth::Error::OAuthTokenSecretNotFoundError:
                    setAuthError(tr("The server's response to a token request provided no token secret."));
                    break;
                case QAbstractOAuth::Error::OAuthCallbackNotVerified:
                    setAuthError(tr("The authorization server has not verified the supplied callback URI in the request."));
                    break;
                default:
                    setAuthError(tr("Authentication failed with (error %1).").arg(int(error)));
                    break;
            }
        });

    m_authError.clear();
    emit stateChanged();

    m_authFlow->grant();
}

void Server::cancelAuthentication()
{
    m_authError = tr("Authentication has been aborted by user.");
    destroyAuthFlow();
    emit stateChanged();
}

void Server::openConnection()
{
    if (m_socket)
        return;

    stopReconnectTimer();

    if (m_authFlow || !m_authenticated) {
        closeConnection();
        return;
    }

    m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);
    connect(m_socket, &QWebSocket::connected, this, &Server::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &Server::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &Server::onTextMessageReceived);

    m_status = tr("Connecting...");
    emit stateChanged();

    m_socket->open(QStringLiteral("ws://127.0.0.1:8080/"));
}

void Server::openNewConnection()
{
    closeConnection();
    openConnection();
}

void Server::closeConnection()
{
    stopReconnectTimer();

    if (!m_socket)
        return;

    m_socket->disconnect(this);
    m_socket->deleteLater();
    m_socket = nullptr;

    m_status = tr("Offline");
    m_connected = false;
    emit stateChanged();
}

void Server::stopReconnectTimer()
{
    m_updateTimer->stop();

    if (m_reconnectTimer) {
        m_reconnectTimer->disconnect(this);
        m_reconnectTimer->deleteLater();
        m_reconnectTimer = nullptr;
    }
}

void Server::setAuthError(const QString& error)
{
    destroyAuthFlow();
    m_authError = error;
    emit stateChanged();
}

void Server::destroyAuthFlow()
{
    if (m_authFlow) {
        m_authFlow->disconnect(this);
        m_authFlow->deleteLater();
        m_authFlow = nullptr;
    }
}

void Server::updateReconnectStatus()
{
    int timeLeft = -1;
    if (m_reconnectTimer)
        timeLeft = m_reconnectTimer->remainingTime();

    if (m_statusTime == timeLeft)
        return;

    m_statusTime = timeLeft;
    if (timeLeft <= 0) {
        m_updateTimer->stop();
        return;
    }

    m_status = tr("Will reconnect in %1s...").arg((timeLeft + 999) / 1000);
    if (!m_error.isEmpty())
        m_status = QStringLiteral("<div style='color:red;font-weight:bold'>%1</div><br>%2").arg(m_error).arg(m_status);

    emit stateChanged();
}

void Server::onConnected()
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    if (socket != m_socket)
        return;

    stopReconnectTimer();

    m_connected = true;
    m_reconnectWait = InitialReconnectWait;
    m_status = tr("Connected");
    emit stateChanged();

    // FIXME
    socket->sendTextMessage(QStringLiteral("Hello, world!"));
}

void Server::onDisconnected()
{
    QWebSocket* socket = qobject_cast<QWebSocket*>(sender());
    if (socket != m_socket)
        return;

    m_error = m_socket->errorString();

    socket->disconnect(this);
    socket->deleteLater();

    m_connected = false;
    m_socket = nullptr;

    stopReconnectTimer();

    if (!m_authenticated)
        return;

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    m_reconnectTimer->callOnTimeout(this, &Server::openConnection);
    m_reconnectTimer->start(m_reconnectWait);

    m_reconnectWait += ReconnectWaitGrowth;
    if (m_reconnectWait > MaxReconnectWait)
        m_reconnectWait = MaxReconnectWait;

    m_statusTime = -2;
    m_updateTimer->start();
    updateReconnectStatus();
}

void Server::onTextMessageReceived(const QString& message)
{
    // FIXME
}
