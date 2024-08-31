#include "Server.h"
#include <QDesktopServices>

const int InitialReconnectWait = 3000;
const int MaxReconnectWait = 30000;
const int ReconnectWaitGrowth = 2000;

#ifdef WASM_TARGET
#include <emscripten.h>
#include <emscripten/val.h>
#endif

#include "ServerOfflineState.h"
#include "ServerAuthGoogleState.h"
#include "ServerConnectingState.h"
#include "ServerWaitingReconnectState.h"
#include "ServerOnlineState.h"

Server::Server(QObject* parent)
    : QObject(parent)
    , m_reconnectWait(InitialReconnectWait)
{
    m_state = new ServerOfflineState(this);
}

Server::~Server()
{
}

void Server::authenticateWithGoogle()
{
    m_lastError.clear();
    m_auth = Auth();
    m_reconnectWait = InitialReconnectWait;
    setState(new ServerAuthGoogleState(this, [this](const QString& token) {
            m_auth.googleToken = token;
            openConnection();
        }));
}

void Server::reconnect()
{
    switch (m_state->id()) {
        case StateID::Offline:
            return; // need to authenticate first
        case StateID::Authenticating:
            return; // already authenticating, do nothing
        case StateID::Connecting:
            break; // already connecting. disconnect and reconnect
        case StateID::Online:
            break; // connected. disconnect and reconnect
        case StateID::WaitingReconnect:
            break; // waiting to reconnect. stop the timer and try immediately
    }

    openConnection();
}

void Server::reconnectLater(const QString& error)
{
    switch (m_state->id()) {
        case StateID::Offline:
            return; // need to authenticate first
        case StateID::Authenticating:
            return; // already authenticating, do nothing
        case StateID::Connecting:
            break; // already connecting. disconnect and reconnect
        case StateID::Online:
            break; // connected. disconnect and reconnect
        case StateID::WaitingReconnect:
            break; // waiting to reconnect. stop the timer and try immediately
    }

    int seconds = m_reconnectWait;

    m_reconnectWait += ReconnectWaitGrowth;
    if (m_reconnectWait > MaxReconnectWait)
        m_reconnectWait = MaxReconnectWait;

    m_lastError = error;
    setState(new ServerWaitingReconnectState(this, seconds));
}

void Server::abortConnection(const QString& error)
{
    m_lastError = error;
    m_reconnectWait = InitialReconnectWait;
    m_auth = Auth();
    setState(new ServerOfflineState(this));
}

void Server::openConnection()
{
    m_lastError.clear();
    setState(new ServerConnectingState(this, QStringLiteral("ws://127.0.0.1:8080/"), m_auth,
        [this](QWebSocket* socket) {
            m_reconnectWait = InitialReconnectWait;
            setState(new ServerOnlineState(this, socket));
        }));
}

void Server::setState(State* state)
{
    m_state->deleteLater();
    m_state = state;
    emit stateChanged();
}

#ifdef WASM_TARGET
EM_JS(void, JSHidePopup, (), {
        if (window.drunkflyAuthPopup) {
            try { window.drunkflyAuthPopup.close(); } catch (e) { console.error(e); }
            window.drunkflyAuthPopup = null;
        }
    });
EM_JS(void, JSShowPopup, (const char* url), {
        if (window.drunkflyAuthPopup) {
            try { window.drunkflyAuthPopup.close(); } catch (e) { console.error(e); }
            window.drunkflyAuthPopup = null;
        }
        window.drunkflyAuthPopup = window.open(UTF8ToString(url), "DrunkFlyAuth", "popup=true,width=700,height=400");
    });
#endif

void Server::State::openUrl(const QString& url)
{
  #ifdef WASM_TARGET
    JSShowPopup(url.toUtf8().constData());
  #else
    QDesktopServices::openUrl(url);
  #endif
}

void Server::State::closeUrl()
{
  #ifdef WASM_TARGET
    JSHidePopup();
  #endif
}
