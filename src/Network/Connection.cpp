#include "Connection.h"
#include "Protobuf/_Packet.qpb.h"
#include <QWebSocket>

const int REQUEST_TIMEOUT = 1000;

Connection::Connection(QWebSocket* socket)
    : QObject(socket)
    , m_socket(socket)
    , m_nextSeq(1)
    , m_iterating(false)
{
    connect(m_socket, &QWebSocket::binaryMessageReceived, this, &Connection::onMessageReceived);
}

Connection::~Connection()
{
}

void Connection::sendPacket(int msgID, const QByteArray& data)
{
    Protobuf::NetPacket packet;
    packet.setSeq(0);
    packet.setMsgID(msgID);
    packet.setPayload(data);
    m_socket->sendBinaryMessage(packet.serialize(&m_serializer));
}

void Connection::sendPacketWithCB(int msgID, const QByteArray& data,
    QObject* r, const IPacketDecoder* decoder, std::function<void(const void*)> cb)
{
    QTimer* timer;
    if (m_freeTimers.size() > 0)
        timer = m_freeTimers.pop();
    else
        timer = new QTimer(this);

    qint64 seq = m_nextSeq++;
    m_pendingCallbacks.emplace(seq, Callback{msgID, decoder, r, cb, timer});

    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [this, seq]{ onResponseReceived(seq, nullptr); });
    timer->start(REQUEST_TIMEOUT);

    Protobuf::NetPacket packet;
    packet.setSeq(seq);
    packet.setMsgID(msgID);
    packet.setPayload(data);
    m_socket->sendBinaryMessage(packet.serialize(&m_serializer));
}

void Connection::registerListener(int msgID,
    QObject* r, const IPacketDecoder* decoder, std::function<void(const void*)> cb)
{
    if (m_iterating) {
        qWarning("Attempted register listener for message while iterating (ID %d).", msgID);
        return;
    }

    ListenerList* listeners;
    auto it = m_listeners.find(msgID);
    if (it == m_listeners.end())
        listeners = &m_listeners.emplace(msgID, ListenerList{decoder, {}}).value();
    else {
        listeners = &it.value();
        size_t n = listeners->list.size();
        while (n-- > 0) {
            if (listeners->list[n].receiver == r) {
                qWarning("Attempted to register duplicate listener for message (ID %d).", msgID);
                return;
            }
        }
    }

    listeners->list.emplace_back(Listener{r, cb});
}

bool Connection::unregisterListener(int msgID, QObject* r)
{
    auto it = m_listeners.find(msgID);
    if (it != m_listeners.end()) {
        QVector<Listener>& listeners = it.value().list;
        size_t n = listeners.size();
        while (n-- > 0) {
            if (listeners[n].receiver == r) {
                if (!m_iterating)
                    listeners.removeAt(n);
                else
                    listeners[n].receiver = nullptr;
                return true;
            }
        }
    }
    return false;
}

void Connection::onMessageReceived(const QByteArray& data)
{
    Protobuf::NetPacket packet;
    packet.deserialize(&m_serializer, data);
    if (m_serializer.deserializationError() != QAbstractProtobufSerializer::NoError) {
        qWarning("Unable to parse server message.");
        emit onSerializationError();
        return;
    }

    auto sequenceID = packet.seq();
    if (sequenceID != 0) {
        onResponseReceived(sequenceID, &packet);
        return;
    }

    auto messageID = packet.msgID();
    auto it = m_listeners.find(messageID);
    if (it == m_listeners.end()) {
        qWarning("Received notification with no listeners (ID %ld).", long(messageID));
        return;
    }
    ListenerList& listeners = it.value();
    size_t n = listeners.list.size();
    if (n == 0) {
        qWarning("Received notification with no active listeners (ID %ld).", long(messageID));
        return;
    }

    void* buf = alloca(listeners.decoder->packetSize());
    if (!listeners.decoder->decode(m_serializer, buf, packet.payload())) {
        qWarning("Unable to parse payload for message (ID %ld).", long(messageID));
        emit onSerializationError();
        return;
    }

    Q_ASSERT(!m_iterating);
    m_iterating = true;
    try {
        while (n-- > 0) {
            Listener& listener = listeners.list[n];
            QObject* receiver = listener.receiver;
            if (receiver == nullptr)
                listeners.list.removeAt(n);
            else
                listener.callback(buf);
        }
    } catch (...) {
        m_iterating = false;
        listeners.decoder->freePacket(buf);
        throw;
    }
    m_iterating = false;
    listeners.decoder->freePacket(buf);
}

void Connection::onResponseReceived(qint64 seq, const Protobuf::NetPacket* packet)
{
    auto it = m_pendingCallbacks.find(seq);
    if (it == m_pendingCallbacks.end()) {
        if (!packet)
            qWarning("Unable to find handler for server response (seq %ld)", long(seq));
        else
            qWarning("Received unexpected response from server (ID %ld, seq %ld)", long(packet->msgID()), long(seq));
        emit onProtocolError();
        return;
    }

    Callback cb = it.value();
    m_pendingCallbacks.erase(it);

    QTimer* timer = cb.timer;
    timer->stop();
    timer->disconnect(this);
    m_freeTimers.push(timer);

    QObject* receiver = cb.receiver;
    if (!receiver) {
        qWarning("Handler for server response is not alive anymore (ID %d, seq %ld)", cb.msgID, long(seq));
        return;
    }

    if (!packet) {
        qWarning("Server request timed out (ID %d, seq %ld)", cb.msgID, long(seq));
        cb.callback(nullptr);
        emit onServerError();
        return;
    }

    int expectedID = cb.msgID * Protobuf::NetConstantsGadget::ProtocolResponseIdMultiplier;
    if (packet->msgID() != expectedID) {
        qWarning("Server has sent an unexpected response (reqID %d, respID %ld (expected %d), seq %ld)",
            cb.msgID, long(packet->msgID()), expectedID, long(seq));
        cb.callback(nullptr);
        emit onProtocolError();
        return;
    }

    void* buf = alloca(cb.decoder->packetSize());
    if (!cb.decoder->decode(m_serializer, buf, packet->payload())) {
        qWarning("Unable to parse payload for message (ID %ld, seq %ld).", long(packet->msgID()), long(seq));
        cb.callback(nullptr);
        emit onSerializationError();
        return;
    }

    try {
        cb.callback(buf);
    } catch (...) {
        cb.decoder->freePacket(buf);
        throw;
    }

    cb.decoder->freePacket(buf);
}
